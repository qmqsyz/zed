#ifndef ZED_FIBER_FIBER_H_
#define ZED_FIBER_FIBER_H_

#include <functional>
#include <memory>

#include <ucontext.h>

namespace zed {

class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {
    friend class Scheduler;

public:
    using Ptr = std::shared_ptr<Fiber>;
    using CallBackFunc = std::function<void()>;

    enum State { INIT, HOLD, EXEC, TERM, READY, EXCEPT };

private:
    Fiber();

public:
    Fiber(CallBackFunc cb, size_t stack_size = 0, bool use_caller = false);

    ~Fiber();

    void reset(CallBackFunc cb);

    void swapIn();

    void swapOut();

    void call();

    void back();

    uint64_t getId() const { return m_id; }

    State getState() const { return m_state; }

public:
    /// @brief set current thread's running fiber
    static void SetThis(Fiber* fiber);

    /// @brief get current thread's running fiber
    static Fiber::Ptr GetThis();

    static void YieldToReady();

    static void YieldToHold();

    static uint64_t TotalFibers();

    static void MainFunc();

    static void CallerMainFunc();

    static uint64_t GetFiberId();

private:
    uint64_t     m_id {0};
    uint32_t     m_stack_size {0};
    State        m_state {INIT};
    ucontext     m_context;
    void*        m_stack {nullptr};
    CallBackFunc m_cb;
};

} // namespace zed

#endif // ZED_FIBER_FIBER_H_