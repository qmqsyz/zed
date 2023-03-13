#include "zed/net/asyn_io.h"
#include "zed/log/log.h"

#include <sys/socket.h>
#include <unistd.h>

namespace zed {

namespace net {

    namespace asyn {

        namespace detail {

            FdEvent* InitFdEvent(int fd)
            {
                auto fd_event = FdManager::GetInstance().getFdEvent(fd);
                fd_event->setExecutor(Executor::GetCurrentExecutor());
                fd_event->setNonBlock();
                return fd_event;
            }

        } // namespace detail

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Read(int fd, void* buf, size_t count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::read(fd, buf, count);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLIN);
            co_return ::read(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Write(int fd, const void* buf, size_t count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::write(fd, buf, count);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLOUT);
            co_return ::write(fd, buf, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t>
        Send(int fd, const void* buf, size_t count, int flags)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::send(fd, buf, count, flags);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLOUT);
            co_return ::send(fd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Recv(int fd, void* buf, size_t count, int flags)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::recv(fd, buf, count, flags);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLIN);
            co_return ::recv(fd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int>
        Connect(int fd, const sockaddr* addr, socklen_t addr_len)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::connect(fd, addr, addr_len);
            if (ret == 0) {
                co_return ret;
            } else if (ret != EINPROGRESS) {
                LOG_DEBUG << "conenct error and errinfo:" << strerror(errno);
                co_return ret;
            }
            bool is_timeout = co_await detail::TimerEventAwaiter(fd_event, EPOLLOUT);
            ret = ::connect(fd, addr, addr_len);
            if ((ret < 0 && errno == EISCONN) || ret == 0) {
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
        Accept(int fd, sockaddr* addr, socklen_t* addr_len, int flags)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::accept4(fd, addr, addr_len, flags);
            if (ret >= 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLIN);
            co_return ::accept4(fd, addr, addr_len, flags);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<int> Close(int fd)
        {
            FdManager::GetInstance().getFdEvent(fd)->remove();
            co_return ::close(fd);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t> Readv(int fd, const struct iovec* vec, int count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::readv(fd, vec, count);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLIN);
            co_return ::readv(fd, vec, count);
        }

        [[CO_AWAIT_HINT]] coroutine::Task<ssize_t>
        Writev(int fd, const struct iovec* vec, int count)
        {
            auto fd_event = detail::InitFdEvent(fd);
            auto ret = ::writev(fd, vec, count);
            if (ret > 0) {
                co_return ret;
            }
            co_await detail::AddEventAwaiter(fd_event, EPOLLOUT);
            co_return ::writev(fd, vec, count);
        }

        // [[CO_AWAIT_HINT]] coroutine::Task<int> Shutdown(int fd, int flag) { }

    } // namespace asyn

} // namespace net

} // namespace zed
