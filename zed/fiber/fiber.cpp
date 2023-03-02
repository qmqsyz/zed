#include "zed/fiber/fiber.h"
#include "zed/config.h"
#include "zed/fiber/scheduler.h"
#include "zed/log/log.h"

#include <atomic>

namespace zed {

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber*     t_fiber {nullptr};
static thread_local Fiber::Ptr t_thread_fiber {nullptr};

static ConfigVar<uint32_t>::Ptr g_fiber_stack_size {
    Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size")};

class StackAllocator {
public:
    static void* Alloc(size_t size) { return ::malloc(size); }
    static void  Dealloc(void* vp, size_t size) { return free(vp); }
};

uint64_t Fiber::GetFiberId()
{
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

Fiber::Fiber()
{
    m_state = EXEC;
    SetThis(this);

    if (::getcontext(&m_context)) {
        LOG_FATAL << "getcontext failed";
        std::terminate();
    }

    ++s_fiber_count;

    LOG_DEBUG << "Fiber::Fiber main construct";
}

Fiber::Fiber(CallBackFunc cb, size_t stack_size = 0, bool use_caller = false)
    : m_id(++s_fiber_id), m_cb(cb)
{
    ++s_fiber_count;
    m_stack_size = stack_size ? stack_size : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stack_size);
    if (::getcontext(&m_context)) {
        LOG_FATAL << "getcontext failed";
        std::terminate();
    }

    m_context.uc_link = nullptr;
    m_context.uc_stack.ss_sp = m_stack;
    m_context.uc_stack.ss_size = m_stack_size;

    if (!use_caller) {
        ::makecontext(&m_context, &Fiber::MainFunc, 0);
    } else {
        ::makecontext(&m_context, &Fiber::CallerMainFunc, 0);
    }

    LOG_DEBUG << "Fiber::Fiber id = " << m_id << " construct";
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if (m_stack) {
        StackAllocator::Dealloc(m_stack, m_stack_size);
    } else {
        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }

    LOG_DEBUG << "Fiber::~Fiber id = " << m_id << " destruct reamin " << s_fiber_count << " fibers";
}

void Fiber::reset(CallBackFunc cb)
{
    m_cb = std::move(cb);
    if (::getcontext(&m_context)) {
        LOG_ERROR << "getcontext failed";
        std::terminate();
    }

    m_context.uc_link = nullptr;
    m_context.uc_stack.ss_sp = m_stack;
    m_context.uc_stack.ss_size = m_stack_size;

    ::makecontext(&m_context, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::swapIn()
{
    SetThis(this);
    m_state = EXEC;
    if (::swapcontext(&Scheduler::GetMainFiber()->m_context, &m_context)) { }
}

void Fiber::swapOut()
{
    SetThis(Scheduler::GetMainFiber());
    if (::swapcontext(&m_context, &Scheduler::GetMainFiber()->m_context)) { }
}

void Fiber::call()
{
    SetThis(this);
    m_state = EXEC;
    if (::swapcontext(&t_thread_fiber->m_context, &m_context)) [[unlinkely]] {
        // TODO
    }
}

void Fiber::back()
{
    SetThis(t_thread_fiber.get());
    if (::swapcontext(&m_context, &t_thread_fiber->m_context)) [[unlinkly]] {
        // TODO
    }
}

void Fiber::SetThis(Fiber* fiber)
{
    t_fiber = fiber;
}
Fiber::Ptr Fiber::GetThis()
{
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::Ptr main_fiber(new Fiber);
    t_thread_fiber = main_fiber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady()
{
    Fiber::Ptr cur {GetThis()};
    cur->m_state = READY;
    cur->swapOut();
}

void Fiber::YieldToHold()
{
    Fiber::Ptr cur = GetThis();
    cur->swapOut();
}

uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}

void Fiber::MainFunc()
{
    Fiber::Ptr cur = GetThis();

    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LOG_ERROR << "Fiber Except: " << ex.what() << " fiber_id = " << cur->getId();
    } catch (...) {
        LOG_ERROR << "Fiber Excep"
                  << " fiber_id = " << cur->getId();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
}

void Fiber::CallerMainFunc()
{
    Fiber::Ptr cur = GetThis();

    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        LOG_ERROR << "Fiber Except: " << ex.what() << " fiber_id = " << cur->getId();
    } catch (...) {
        LOG_ERROR << "Fiber Excep"
                  << " fiber_id = " << cur->getId();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
}

} // namespace zed
