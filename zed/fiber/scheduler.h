#ifndef ZED_FIBER_SCHEDULER_H_
#define ZED_FIBER_SCHEDULER_H_

#include <memory>
#include <mutex>
#include <vector>

#include "zed/fiber/fiber.h"
#include "zed/thread.h"

namespace zed {

class Scheduler {

public:
    using Ptr = std::shared_ptr<Scheduler>;

    Scheduler(size_t thread_num, bool use_caller = true, const std::string& name = "");

    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    void start();

    void stop();

    template <class FiberCB>
    void schedule(FiberCB f, int thread = -1)
    {
        bool need_tickle = false;
        {
            std::lock_gaurd lock(m_mutex);
            need_tickle = scheduWithNoLock(f, thread);
        }
        if (need_tickle) {
            tickle();
        }
    }

    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if (need_tickle) {
            tickle();
        }
    }

    void switchTo(int thread = -1);

    std::ostream& dump(std::ostream& os);

public:
    static Scheduler* GetCurrentScheduler();

    static Fiber* GetMainFiber();

protected:
    virtual void tickle();

    void run();

    virtual bool stopping();

    virtual void idle();

    void setThis();

    bool hasIdleThreads() const { return m_idle_thread_count > 0; }

private:
    struct FiberAndThread {
        Fiber::Ptr            fiber;
        std::function<void()> cb;
        int                   thread_id;

        FiberAndThread(Fiber::Ptr f, int thread_id) : fiber(f), thread_id(thread_id) { }

        FiberAndThread(Fiber::Ptr* f, int thread_id) : thread_id(thread_id) { fiber.swap(*f); }

        FiberAndThread(std::function<void()> f, int thread_id) : cb(f), thread_id(thread_id) { }

        FiberAndThread(std::function<void()>* f, int thread_id) : thread_id(thread_id)
        {
            cb.swap(*f);
        }

        FiberAndThread() : thread_id(-1) { }

        void reset()
        {
            fiber = nullptr;
            cb = nullptr;
            thread_id = -1;
        }
    };

    template <class FiberCB>
    bool scheduleNoLock(FiberCB f, int thread)
    {
        bool need_tickle = m_fibers.empty();

        FiberAndThread ft(f, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }

private:
    std::mutex                m_mutex;
    std::vector<Thread::Ptr>  m_threads;
    std::list<FiberAndThread> m_fibers;
    Fiber::Ptr                m_root_fiber;
    std::string               m_name;

protected:
    std::vector<int>    m_threadIDs;
    size_t              m_thread_count {0};
    std::atomic<size_t> m_active_thread_count {0};
    std::atomic<size_t> m_idle_thread_count {0};
    bool                m_stopping {true};
    bool                m_auto_stop {false};
    int                 m_root_thread_id {0};
};

class SchedulerSwitcher : public Noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();

private:
    Scheduler* m_caller;
};

} // namespace zed

#endif // ZED_FIBER_SCHEDULER_H_