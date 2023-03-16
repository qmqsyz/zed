#ifndef ZED_NET_EXECUTOR_H_
#define ZED_NET_EXECUTOR_H_

#include "zed/comm/thread.h"
#include "zed/coroutine/task.hpp"
#include "zed/util/singleton.hpp"

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <sys/epoll.h>
#include <unordered_set>

namespace zed {

namespace net {

    class Timer;
    class FdEvent;

    enum ExecutorType {
        Main,
        Sub,
    };

    class Executor {
    public:
        using Func = std::function<void()>;

        explicit Executor();

        ~Executor();

        void updateEvent(int fd, epoll_event event, bool is_wakeup = true);

        void delEvent(int fd, bool is_wakeup = true);

        void addTask(coroutine::Task<>&& task, bool is_wakeup = true);

        void addTask(std::function<void()> task, bool is_wakeup = true);

        void start();

        void stop();

        Timer* getTimer();

        pid_t getTid() const { return m_tid; }

        void setReactorType(ExecutorType type) noexcept { m_executor_type = type; }

    public:
        static Executor* GetCurrentExecutor() noexcept;

        static int GetEpollTimeout() noexcept;

    private:
        void wakeup();

        bool isInLoopThread() const { return m_tid == this_thread::GetId(); }

        void updateEventInLoopThread(int fd, epoll_event event);

        void delEventInLoopThread(int fd);

        void consumeCoroutines();

        void consumePenddingTasks();

        // void doInitHandle();

        // void destroyHanlde();

    private:
        int                     m_epoll_fd {-1};
        int                     m_wake_fd {-1};
        int                     m_timer_fd {-1};
        bool                    m_stop_flag {true};
        pid_t                   m_tid {0};
        std::unique_ptr<Timer>  m_timer {nullptr};
        std::unordered_set<int> m_fds;
        std::mutex              m_mutex {};
        std::vector<Func>       m_pendding_tasks {};

        // std::mutex m_handles_mutex {};
        // std::vector<std::coroutine_handle<>> m_init_handles {};
        // std::list<std::coroutine_handle<>>   m_remain_handles {};

        ExecutorType m_executor_type {Sub};
    };

    class TaskQueue {
    public:
        void push(FdEvent* fd);

        FdEvent* take();

    private:
        std::queue<FdEvent*> m_tasks {};
        std::mutex           m_mutex {};
    };

    using TaskManager = util::Singleton<TaskQueue>;

} // namespace net

} // namespace zed

#endif // ZED_NET_EXECUTOR_H_