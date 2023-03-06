#ifndef ZED_NET_TIMER_H_
#define ZED_NET_TIMER_H_

#include "zed/net/fd_event.h"

#include <functional>
#include <memory>
#include <set>
#include <shared_mutex>

namespace zed {

namespace net {

    int64_t GetNowMicroSecond();

    class Timer;
    class Executor;

    class TimerEvent {
    public:
        using Ptr = std::shared_ptr<TimerEvent>;
        using Func = std::function<void()>;

        friend class Timer;

        /// @brief the constructer only for onTimer use
        explicit TimerEvent(int64_t now) : m_expired_time(now) { }

        TimerEvent(int64_t interval, bool is_repeated, Func task)
            : m_interval(interval), m_is_repeated(is_repeated), m_task(std::move(task))
        {
            m_expired_time = GetNowMicroSecond() + m_interval * 1000;
        }

        void resetTime()
        {
            m_expired_time = GetNowMicroSecond() + m_interval;
            m_is_canceled = false;
        }

        void wake() { m_is_canceled = false; }

        void cancel() { m_is_canceled = true; }

        void cancelRepeated() { m_is_repeated = false; }

    private:
        struct Comparator {
            bool operator()(const TimerEvent::Ptr& lhs, const TimerEvent::Ptr& rhs) const;
        };

    private:
        int64_t m_expired_time {0};
        int64_t m_interval {0};
        bool    m_is_repeated {false};
        bool    m_is_canceled {false};
        Func    m_task {};
    };

    class Timer final : public FdEvent {
    public:
        using Ptr = std::shared_ptr<Timer>;

        Timer(Executor* executor);

        ~Timer();

        void addTimerEvent(TimerEvent::Ptr event, bool need_reset = true);

        void delTimerEvent(TimerEvent::Ptr event);

        void resetExpiredTime();

        void onTimer();

    private:
        std::set<TimerEvent::Ptr> m_pending_events;
        std::shared_mutex         m_rwmutex;
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_TIMER_H_
