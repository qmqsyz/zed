#ifndef ZED_NET_LAZYIO_H_
#define ZED_NET_LAZYIO_H_

#include "zed/coroutine/hint.hpp"
#include "zed/coroutine/task.hpp"
#include "zed/net/executor.h"
#include "zed/net/fd_event.h"
#include "zed/net/timer.h"

namespace zed {

namespace net {

    namespace lazy {

        namespace detail {

            struct EventAwaiter {
                FdEvent* m_fd_event;
                int      m_events;

                EventAwaiter(FdEvent* fd_event, int events) noexcept
                    : m_fd_event(fd_event), m_events(events)
                {
                }

                constexpr bool await_ready() noexcept { return false; }

                std::suspend_always await_suspend(std::coroutine_handle<> who_call_me)
                {
                    m_fd_event->setHandle(who_call_me);
                    m_fd_event->addEvents(m_events);
                }

                constexpr void await_resume() noexcept
                {
                    m_fd_event->delEvents(m_events);
                    m_fd_event->setHandle(nullptr);
                }
            };

            struct TimerEventAwaiter {
                FdEvent*        m_fd_event;
                int             m_events;
                bool            is_timeout {false};
                TimerEvent::Ptr m_timer_event {};

                EventAwaiter(FdEvent* fd_event, int events) noexcept
                    : m_fd_event(fd_event), m_events(events)
                {
                }
                constexpr bool await_ready() noexcept { return false; }

                std::suspend_always await_suspend(std::coroutine_handle<> who_call_me)
                {
                    auto task = [&is_timeout, who_call_me]() {
                        is_timeout = true;
                        who_call_me.resume();
                    };
                    m_timer_event = std::make_shared<TimerEvent>(t_max_epoll_timeout, false, task);
                    Executor::GetCurrentExecutor()->getTimer().addTimerEvent(timer_event);
                    m_fd_event->setHandle(who_call_me);
                    m_fd_event->addEvents(m_events);
                }

                bool await_resume() noexcept
                {
                    Executor::GetCurrentExecutor()->getTimer().delTimerEvent(m_timer_event);
                    m_fd_event->delEvents(m_events);
                    m_fd_event->setHandle(nullptr);
                    return is_timeout;
                }
            };

            FdEvent* InitFdEvent(int fd)
            {
                auto fd_event = FdManager::GetInstance().getFdEvent(fd);
                fd_event->setExecutor(Executor::GetCurrentExecutor());
                fd_event->setNonBlock();
                return fd_event;
            }

        } // namespace detail

        [[CO_AWAIT_HINT]] coroutine::Task<int> Read(int fd, void* buf, size_t count)
        {
            auto   fd_event = detail::InitFdEvent(fd);
            size_t n = ::read(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await EventAwaiter(fd_event, EPOLLIN);
            co_return ::read(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Write(int fd, void* buf, size_t count)
        {
            auto   fd_event = detail::InitFdEvent(fd);
            size_t n = ::write(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await EventAwaiter(fd_event, EPOLLOUT);
            co_return ::write(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Send(int fd, void* buf, size_t count)
        {
            auto   fd_event = detail::InitFdEvent(fd);
            size_t n = ::send(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await EventAwaiter(fd_event, EPOLLOUT);
            co_return ::send(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Recv(int fd, void* buf, size_t count)
        {
            auto   fd_event = detail::InitFdEvent(fd);
            size_t n = ::recv(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await EventAwaiter(fd_event, EPOLLIN);
            co_return ::recv(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
        {
            auto fd_event = detail::InitFdEvent(fd);
            int  ret = ::connect(sockfd, addr, addrlen);
            if (ret == 0) {
                co_return ret;
            } else if (ret != EINPROGRESS) {
                LOG_DEBUG << "conenct error and errinfo:" << strerror(errno);
                co_return ret;
            }
            bool is_timeout = co_await detail::TimerEventAwaiter(fd_event, EPOLLOUT);
            int  n = ::connect(sockfd, addr, addrlen);
            if ((n < 0 && errno == EISCONN) || n == 0) {
                LOG_DEBUG << "connect successed";
                return 0;
            }
            if (is_timeout) {
                LOG_ERROR << "connect timeout";
                errno = ETIMOUT;
            }
            LOG_DEBUG << "connect failed errinfo:" << strerror(errno);
            return -1;
        }

    } // namespace lazy

} // namespace net

} // namespace zed

#endif // ZED_COROUTINE_NET_H_