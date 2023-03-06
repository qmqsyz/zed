#include "zed/net/executor.h"
#include "zed/log/log.h"
#include "zed/net/timer.h"

#include <sys/eventfd.h>

namespace zed {

namespace net {

    static thread_local Executor* t_executor {nullptr};

    static thread_local int t_max_epoll_timeout {10000};

    Executor* Executor::GetCurrentExecutor()
    {
        return t_executor;
    }

    Executor::Executor()
        : m_tid {Thread::GetCurrentThreadId()}
        , m_epoll_fd {epoll_create1(EPOLL_CLOEXEC)}
        , m_wake_fd(eventfd(0, EFD_NONBLOCK))
    {
        if (t_executor != nullptr) [[unlikely]] {
            LOG_ERROR << "to many executor on a thread! threadname =  "
                      << Thread::GetCurrentThreadName;
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
        event.events = EPOLLIN;
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
        ::close(m_epoll_fd);
        ::close(m_wake_fd);
        m_timer.reset();

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
            LOG_ERROR << "epoll_ctl failed fd = " << fd << " sys errinfo = " << strerror(errno);
            return;
        }
        if (is_add) {
            m_fds.emplace(fd);
        }
        LOG_DEBUG << (is_add ? " add " : " mod ") << "fd = " << fd << " successfully";
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
            LOG_DEBUG << "epoll_ctl failed fd = " << fd << " sys errinfo = " << strerror(errno);
        }
        m_fds.erase(it);
        LOG_DEBUG << "delete fd = " << fd << " successfully";
    }

    void Executor::schedule(coroutine::Task<>&& task, bool is_wakeup)
    {
        auto handle = task.getHandle();
        task.detach();
        {
            std::lock_guard lock(m_handles_mutex);
            m_init_handles.emplace_back(std::move(handle));
        }
        // LOG_DEBUG << "schedule a handle";
        if (is_wakeup) [[likely]] {
            wakeup();
        }
    }

    void Executor::addTask(std::function<void()> task, bool is_wakeup)
    {
        std::lock_guard lock(m_mutex);
        m_pendding_tasks.emplace_back(std::move(task));
        LOG_DEBUG << "add task successfully";
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

            doInitHandle();
            LOG_DEBUG << "finished doInitHandle";
            if (m_executor_type == Sub) {
                consumeCoroutines();
                LOG_DEBUG << "finished consumeCoroutines";
            }

            doRemainHanlde();
            LOG_DEBUG << "finished doRemainHandle";

            if (first_handle) {
                first_handle.resume();
                first_handle = nullptr;
            }

            consumePenddingTasks();
            LOG_DEBUG << "finished consumePenddingTask";

            int cnt = epoll_wait(m_epoll_fd, re_events, MAX_EVENTS, t_max_epoll_timeout);

            LOG_DEBUG << "epoll_wait return " << cnt;

            if (cnt < 0) {
                LOG_ERROR << "epoll_wait failed errinfo = " << strerror(errno);
            } else {
                for (int i {0}; i < cnt; ++i) {
                    epoll_event event = re_events[i];

                    if (event.data.fd == m_wake_fd && (event.events & EPOLLIN)) {
                        char buf[8];
                        if (read(m_wake_fd, buf, sizeof(buf)) != sizeof(8)) {
                            LOG_DEBUG << "read wake_fd failed errinfo = " << strerror(errno);
                        }
                    } else {
                        FdEvent* ptr = static_cast<FdEvent*>(event.data.ptr);
                        if (ptr == nullptr) [[unlikely]] {
                            continue;
                        }
                        if (ptr->getHandle()) {
                            if (m_executor_type == Sub) [[likely]] { // sub more than main
                                if (first_handle == nullptr) {
                                    first_handle = ptr->getHandle();
                                } else {
                                    delEventInLoopThread(event.data.fd);
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
                // LOG_DEBUG << "resume a handle fd [ " << ptr->getFd() << " ]";
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
            task();
        }
    }

    void Executor::doInitHandle()
    {
        std::vector<std::coroutine_handle<>> handle_tmp;
        {
            std::lock_guard lock(m_handles_mutex);
            handle_tmp.swap(m_init_handles);
        }

        for (auto handle : handle_tmp) {
            handle.resume();
            m_remain_handles.emplace_back(std::move(handle));
        }
    }

    // do not lock only loop can execute the func
    void Executor::doRemainHanlde()
    {
        std::vector<std::coroutine_handle<>> handle_tmp;
        handle_tmp.swap(m_remain_handles);
        for (auto handle : handle_tmp) {
            if (handle.done()) {
                // LOG_DEBUG << "destroy a handle";
                handle.destroy();
            } else {
                // LOG_DEBUG << "handle is no't finished";
                m_remain_handles.emplace_back(std::move(handle));
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
