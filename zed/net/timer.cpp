#include "zed/net/timer.h"
#include "zed/log/log.h"

#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/timerfd.h>

namespace zed {

namespace net {

    bool TimerEvent::Comparator::operator()(const TimerEvent::Ptr& lhs,
                                            const TimerEvent::Ptr& rhs) const
    {
        if (!lhs && !rhs) {
            return false;
        }
        if (!lhs) {
            return true;
        }
        if (!rhs) {
            return false;
        }
        if (lhs->m_expired_time < rhs->m_expired_time) {
            return true;
        }
        if (rhs->m_expired_time > rhs->m_expired_time) {
            return false;
        }
        return lhs.get() < rhs.get();
    }

    int64_t GetNowMicroSecond()
    {
        timeval val;
        ::gettimeofday(&val, nullptr);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    Timer::Timer(Executor* executor)
        : FdEvent(executor, ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK))
    {
        if (m_fd < 0) {
            LOG_ERROR << "timerfd_create failed";
        }

        m_read_callback = std::bind(&Timer::onTimer, this);
        addEvents(EPOLLIN);
    }

    Timer::~Timer()
    {
        remove();
        ::close(m_fd);
    }

    void Timer::addTimerEvent(TimerEvent::Ptr event, bool need_reset)
    {
        bool is_reset = false;
        {
            std::lock_guard lock(m_rwmutex);
            if (m_pending_events.empty()) {
                is_reset = true;
            } else {
                auto it = m_pending_events.begin();
                if (event->m_expired_time < (*it)->m_expired_time) {
                    is_reset = true;
                }
            }
            m_pending_events.emplace(event);
            // LOG_DEBUG << "Timer::addTimerEvent ok";
        }
        if (is_reset && need_reset) {
            resetExpiredTime();
        }
    }

    void Timer::delTimerEvent(TimerEvent::Ptr event)
    {
        event->m_is_canceled = true;
        std::lock_guard lock(m_rwmutex);

        auto it = m_pending_events.find(event);
        if (it != m_pending_events.end()) [[likely]] {
            m_pending_events.erase(it);
        }
    }

    void Timer::resetExpiredTime()
    {
        std::shared_lock lock(m_rwmutex);

        auto it = m_pending_events.begin();

        lock.unlock();

        if (it == m_pending_events.end()) {
            LOG_DEBUG << "no timer event pending";
            return;
        }

        int64_t now = GetNowMicroSecond();

        if ((*it)->m_expired_time < now) {
            LOG_DEBUG << "don't reset the expired time";
            return;
        }

        int64_t interval = (*it)->m_expired_time - now;

        itimerspec new_value;
        memset(&new_value, 0, sizeof(new_value));

        timespec ts;
        memset(&ts, 0, sizeof(ts));
        ts.tv_sec = interval / 1000;
        ts.tv_nsec = (interval % 1000) * 1'000'000;
        new_value.it_value = ts;

        int ret = timerfd_settime(m_fd, 0, &new_value, nullptr);
        if (ret != 0) [[unlikely]] {
            LOG_ERROR << "timerfd_settime failed";
        }
    }

    void Timer::onTimer()
    {
        char buf[8];
        if (int n = ::read(m_fd, buf, sizeof(buf)); n != sizeof(buf)) {
            LOG_ERROR << "errinfo" << errno << " " << strerror(errno) << " " << n << " " << m_fd;
        }

        std::vector<TimerEvent::Ptr> expired_timer_events;
        {
            std::lock_guard lock(m_rwmutex);

            int64_t now = GetNowMicroSecond();

            auto tmp = std::make_shared<TimerEvent>(now);

            auto it = m_pending_events.upper_bound(tmp);

            expired_timer_events.insert(expired_timer_events.end(), m_pending_events.begin(), it);
            m_pending_events.erase(m_pending_events.begin(), it);
        }

        std::vector<std::function<void()>> tasks;
        for (auto& event : expired_timer_events) {
            if (event->m_is_canceled) {
                continue;
            } else if (event->m_is_repeated) {
                event->resetTime();
                addTimerEvent(event, false);
            }
            tasks.emplace_back(event->m_task);
        }

        resetExpiredTime();

        for (const auto& task : tasks) {
            task();
        }
    }

} // namespace net

} // namespace zed
