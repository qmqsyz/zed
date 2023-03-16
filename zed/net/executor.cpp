#include "zed/net/executor.h"
#include "zed/log/log.h"
#include "zed/net/timer.h"

#include <sys/eventfd.h>

namespace zed {

namespace net {

    static thread_local Executor* t_executor {nullptr};

    static thread_local int t_max_epoll_timeout {10000};

    Executor* Executor::GetCurrentExecutor() noexcept
    {
        return t_executor;
    }

    int Executor::GetEpollTimeout() noexcept
    {
        return t_max_epoll_timeout;
    }

    Executor::Executor()
        : m_tid {this_thread::GetId()}
        , m_epoll_fd {epoll_create1(EPOLL_CLOEXEC)}
        , m_wake_fd(eventfd(0, EFD_NONBLOCK))
    {
        if (t_executor != nullptr) [[unlikely]] {
            LOG_ERROR << "to many executor on a thread! thread_id = " << this_thread::GetId();
            std::terminate();
        }
        t_executor = this;

        if (m_epoll_fd < 0) {
            LOG_ERROR << "epoll_create failed fd = " << m_epoll_fd;
            std::terminate();
        }
        if (m_wake_fd < 0) {
            LOG_ERROR << "eventfd failed fd = " << m_wake_fd;
            std::terminate();
        }

        struct epoll_event event;
        event.data.fd = m_wake_fd;
        event.events = EPOLLIN | EPOLLET;
        if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wake_fd, &event) != 0) {
            LOG_ERROR << "epoll_ctl add m_wake_fd failed fd = " << m_wake_fd;
        }
        m_fds.insert(m_wake_fd);

        LOG_DEBUG << "executor construct successfully";
    }

    Executor::~Executor()
    {
        if (m_stop_flag == false) {
            stop();
        }
        // first timer,second wake_fd ,final epoll_fd;
        m_timer.reset();
        ::close(m_wake_fd);
        ::close(m_epoll_fd);

        t_executor = nullptr;
        LOG_DEBUG << "~Executor";
    }

    void Executor::wakeup()
    {
        if (m_stop_flag == true) {
            return;
        }
        uint64_t one = 1;
        if (write(m_wake_fd, &one, sizeof(one)) != sizeof(one)) {
            LOG_ERROR << "write wake_fd failed sysinfo = " << strerror(errno);
        }
        // LOG_DEBUG << "wakeup once";
    }

    void Executor::updateEvent(int fd, epoll_event event, bool is_wakeup)
    {
        if (fd < 0) {
            LOG_ERROR << "update event failed because invalid fd = " << fd;
            return;
        }
        if (isInLoopThread()) {
            updateEventInLoopThread(fd, event);
            return;
        }
        {
            std::lock_guard lock(m_mutex);
            m_pendding_tasks.emplace_back(
                std::bind(&Executor::updateEventInLoopThread, this, fd, event));
        }
        if (is_wakeup) {
            wakeup();
        }
    }

    void Executor::delEvent(int fd, bool is_wakeup)
    {
        if (fd < 0) {
            LOG_ERROR << "delete event failed because invalid fd = " << fd;
            return;
        }
        if (isInLoopThread()) {
            delEventInLoopThread(fd);
            return;
        }
        {
            std::lock_guard lock(m_mutex);
            m_pendding_tasks.emplace_back(std::bind(&Executor::delEventInLoopThread, this, fd));
        }
        if (is_wakeup) {
            wakeup();
        }
    }

    void Executor::updateEventInLoopThread(int fd, epoll_event event)
    {
        assert(isInLoopThread());

        int  operation = EPOLL_CTL_ADD;
        bool is_add = true;
        auto it = m_fds.find(fd);
        if (it != m_fds.end()) {
            is_add = false;
            operation = EPOLL_CTL_MOD;
        }

        if (epoll_ctl(m_epoll_fd, operation, fd, &event) != 0) {
            LOG_ERROR << "epoll_ctl" << (is_add ? " add " : " mod ") << " failed fd = " << fd
                      << " sys errinfo = " << strerror(errno);
            return;
        }
        if (is_add) {
            m_fds.emplace(fd);
        }
        LOG_DEBUG << (is_add ? " add " : " mod ") << "fd [ " << fd << " ] successfully";
    }

    void Executor::delEventInLoopThread(int fd)
    {
        assert(isInLoopThread());

        auto it = m_fds.find(fd);
        if (it == m_fds.end()) [[unlikely]] {
            LOG_DEBUG << "fd [ " << fd << " ] doesn't exist in this executor";
            return;
        }
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) != 0) {
            LOG_ERROR << "epoll_ctl failed fd = " << fd << " sys errinfo = " << strerror(errno);
        }
        m_fds.erase(it);
        // LOG_DEBUG << "delete fd = " << fd << " successfully";
    }

    void Executor::addTask(coroutine::Task<>&& task, bool is_wakeup)
    {
        auto handle = task.getHandle();
        task.detach();
        auto func = [handle = std::move(handle)]() { handle.resume(); };
        addTask(func, is_wakeup);
        // {
        //     std::lock_guard lock(m_handles_mutex);
        //     m_init_handles.emplace_back(std::move(handle));
        // }
        // // LOG_DEBUG << "add a task";
        // if (is_wakeup) [[likely]] {
        //     wakeup();
        // }
    }

    void Executor::addTask(std::function<void()> task, bool is_wakeup)
    {
        std::lock_guard lock(m_mutex);
        m_pendding_tasks.emplace_back(std::move(task));
        // LOG_DEBUG << "add task successfully";
        if (is_wakeup) [[likely]] {
            wakeup();
        }
    }

    void Executor::start()
    {
        assert(isInLoopThread());

        if (m_stop_flag == false) {
            return;
        }

        m_stop_flag = false;

        const int   MAX_EVENTS = 10;
        epoll_event re_events[MAX_EVENTS + 1];

        std::coroutine_handle<> first_handle {nullptr};

        LOG_DEBUG << "executor start!";
        while (m_stop_flag == false) {

            // doInitHandle();

            consumePenddingTasks();
            // LOG_DEBUG << "finished consumePenddingTask";

            int cnt = epoll_wait(m_epoll_fd, re_events, MAX_EVENTS, t_max_epoll_timeout);

            LOG_DEBUG << cnt << " events happened";

            if (cnt < 0) {
                LOG_ERROR << "epoll_wait failed errinfo = " << strerror(errno);
            } else {
                for (int i {0}; i < cnt; ++i) {
                    epoll_event event = re_events[i];

                    if (event.data.fd == m_wake_fd && (event.events & EPOLLIN)) {
                        char buf[8];
                        if (read(m_wake_fd, buf, sizeof(buf)) != sizeof(buf)) {
                            LOG_DEBUG << "read wake_fd failed errinfo = " << strerror(errno);
                        }
                    } else {
                        FdEvent* ptr = static_cast<FdEvent*>(event.data.ptr);
                        auto     fd = ptr->getFd();
                        if (ptr == nullptr) [[unlikely]] {
                            continue;
                        }
                        if (ptr->getHandle()) {
                            if (m_executor_type == Sub) [[likely]] { // sub more than main
                                if (first_handle == nullptr) {
                                    first_handle = ptr->getHandle();
                                } else {
                                    delEventInLoopThread(fd);
                                    // Excutor of FdEvent will change when it be used
                                    TaskManager::GetInstance().push(ptr);
                                }
                            } else {
                                ptr->getHandle().resume();
                            }
                        } else {
                            ptr->handleEvent(event.events);
                        }
                    }
                }
            }

            if (first_handle) {
                first_handle.resume();
                first_handle = nullptr;
            }

            if (m_executor_type == Sub) {
                consumeCoroutines();
            }

            // destroyHanlde();
        }
    }

    void Executor::stop()
    {
        if (m_stop_flag == false) {
            m_stop_flag = true;
            wakeup();
            LOG_DEBUG << "Executor stop";
        }
    }

    Timer* Executor::getTimer()
    {
        if (m_timer == nullptr) [[unlikely]] {
            m_timer = std::make_unique<Timer>(this);
            m_timer_fd = m_timer->getFd();
            m_fds.emplace(m_timer_fd);
            LOG_DEBUG << "build timer event fd [ " << m_timer_fd << " ]";
        }
        return m_timer.get();
    }

    void Executor::consumeCoroutines()
    {
        while (true) {
            auto ptr = TaskManager::GetInstance().take();
            if (ptr == nullptr) [[unlikely]] {
                break;
            } else {
                ptr->setExecutor(this);
                auto handle = ptr->getHandle();
                handle.resume();
            }
        }
    }

    void Executor::consumePenddingTasks()
    {
        std::vector<std::function<void()>> tasks;
        {
            std::lock_guard lock(m_mutex);
            tasks.swap(m_pendding_tasks);
        }
        for (const auto& task : tasks) {
            try {
                task();
            } catch (const std::exception& ex) {
                LOG_ERROR << ex.what();
            }
        }
    }

    void TaskQueue::push(FdEvent* task)
    {
        std::lock_guard lock(m_mutex);
        m_tasks.emplace(task);
    }

    FdEvent* TaskQueue::take()
    {
        FdEvent* task {nullptr};
        {
            std::lock_guard lock(m_mutex);
            if (!m_tasks.empty()) {
                task = m_tasks.front();
                m_tasks.pop();
            }
        }
        return task;
    }

} // namespace net

} // namespace zed
