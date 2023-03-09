#include "zed/net/lazy_io.h"
#include "zed/log/log.h"

#include <sys/socket.h>

namespace zed {

namespace net {

    namespace lazy {

        namespace detail {

            FdEvent* InitFdEvent(int fd)
            {
                auto fd_event = FdManager::GetInstance().getFdEvent(fd);
                fd_event->setExecutor(Executor::GetCurrentExecutor());
                fd_event->setNonBlock();
                // LOG_DEBUG << "init FdEvent";
                return fd_event;
            }

        } // namespace detail

        [[CO_AWAIT_HINT]] coroutine::Task<int> Read(int fd, void* buf, size_t count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            int  n = ::read(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await detail::EventAwaiter(fd_event, EPOLLIN);
            co_return ::read(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Write(int fd, void* buf, size_t count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            int  n = ::write(fd, buf, count);
            if (n > 0) {
                co_return n;
            }
            co_await detail::EventAwaiter(fd_event, EPOLLOUT);
            co_return ::write(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Send(int fd, void* buf, size_t count, int flags)
        {
            auto fd_event = detail::InitFdEvent(fd);
            int  n = ::send(fd, buf, count, flags);
            if (n > 0) {
                co_return n;
            }
            co_await detail::EventAwaiter(fd_event, EPOLLOUT);
            co_return ::send(fd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Recv(int fd, void* buf, size_t count, int flags)
        {
            auto fd_event = detail::InitFdEvent(fd);
            int  n = ::recv(fd, buf, count, flags);
            if (n > 0) {
                co_return n;
            }
            co_await detail::EventAwaiter(fd_event, EPOLLIN);
            co_return ::recv(fd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Connect(int sockfd, const sockaddr* addr, socklen_t addrlen)
        {
            auto fd_event = detail::InitFdEvent(sockfd);
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
                co_return 0;
            }
            if (is_timeout) {
                LOG_ERROR << "connect timeout";
                errno = ETIMEDOUT;
            }
            LOG_DEBUG << "connect failed errinfo:" << strerror(errno);
            co_return -1;
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Accept(int sockfd, sockaddr* addr, socklen_t* addrlen)
        {
            auto fd_event = detail::InitFdEvent(sockfd);
            int  n = ::accept(sockfd, addr, addrlen);
            if (n > 0) {
                co_return n;
            }
            co_await detail::EventAwaiter(fd_event, EPOLLIN);
            co_return ::accept(sockfd, addr, addrlen);
        }

    } // namespace lazy

} // namespace net

} // namespace zed
