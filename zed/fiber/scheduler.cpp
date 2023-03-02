#include "zed/fiber/scheduler.h"
#include "zed/log/log.h"

namespace zed {

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber*     t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name) : m_name(name)
{
    if (use_caller) {
        Fiber::GetThis();
        --threads;

        t_scheduler = this;

        m_root_fiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));

        t_scheduler_fiber = m_root_fiber.get();
        m_root_thread_id = current_thread::GetTid();

        m_threadIDs.push_back(m_root_thread_id);
    } else {
        m_root_thread_id = -1;
    }
    m_thread_count = threads;
}

Scheduler::~Scheduler()
{
    if (GetCurrentScheduler() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetCurrentScheduler()
{
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber()
{
    return t_scheduler_fiber;
}

void Scheduler::start()
{
    std::lock_guard lock(m_mutex);
    if (m_stopping == false) [[unlinkly]] {
        return;
    }
    m_stopping = false;

    m_threads.resize(m_thread_count);
    for (size_t i {0}; i < m_thread_count; ++i) {
        m_threads[i].reset(
            new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIDs.push_back(m_threads[i]->getId());
    }
}

void Scheduler::stop()
{
    m_auto_stop = true;
    if (m_root_fiber && m_thread_count == 0
        && (m_root_fiber->getState() == Fiber::TERM || m_root_fiber->getState() == Fiber::INIT)) {
        LOG_INFO << this << "stopped";
        m_stopping = true;
        if (stopping()) {
            return;
        }
    }

    m_stopping = true;
    for (size_t i = 0; i < m_thread_count; ++i) {
        tickle();
    }

    if (m_root_fiber) {
        tickle();
    }

    if (m_root_fiber) {
        if (!stopping()) {
            m_root_fiber->call();
        }
    }

    std::vector<Thread::Ptr> threads;
    {
        std::lock_guard lock(m_mutex);
        threads.swap(m_threads);
    }
}

void Scheduler::run()
{
    LOG_DEBUG << m_name << " run";
    // set_hook_enable(true);
    setThis();
    if (current_thread::GetTid() != m_root_thread_id) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    Fiber::Ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::Ptr cb_fiber;

    FiberAndThread ft;
    while (true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            std::lock_guard lock(m_mutex);

            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                if (it->thread_id != -1 && it->thread_id != current_thread::GetTid()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_active_thread_count;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }
        if (tickle_me) {
            tickle();
        }
        if (ft.fiber
            && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_active_thread_count;

            if (ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if (ft.fiber->getState() != Fiber::TERM
                       && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if (ft.cb) {
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_active_thread_count;
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if (cb_fiber->getState() == Fiber::EXCEPT
                       || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            if (is_active) {
                --m_active_thread_count;
                continue;
            }
            if (idle_fiber->getState() == Fiber::TERM) {
                LOG_INFO << "idel fiber term";
                break;
            }

            ++m_idle_thread_count;
            idle_fiber->swapIn();
            --m_idle_thread_count;
            if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::setThis()
{
    t_scheduler = this;
}

void Scheduler::tickle()
{
    LOG_INFO << "tickle";
}

bool Scheduler::stopping()
{
    std::lock_guard lock(m_mutex);
    return m_auto_stop && m_stopping && m_fibers.empty() && m_active_thread_count == 0;
}

void Scheduler::idle()
{
    LOG_INFO << "idle";
    while (!stopping()) {
        Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(int thread)
{
    if (Scheduler::GetCurrentScheduler() == this) {
        if (thread == -1 || thread == current_thread::GetTid()) {
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os)
{
    os << "[Scheduler name=" << m_name << " size=" << m_thread_count
       << " active_count=" << m_active_thread_count << " idle_count=" << m_idle_thread_count
       << " stopping=" << m_stopping << " ]" << std::endl
       << "    ";
    for (size_t i = 0; i < m_threadIDs.size(); ++i) {
        if (i) {
            os << ", ";
        }
        os << m_threadIDs[i];
    }
    return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target)
{
    m_caller = Scheduler::GetCurrentScheduler();
    if (target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher()
{
    if (m_caller) {
        m_caller->switchTo();
    }
}

} // namespace zed
