#include "zed/net/fd_event.h"
#include "zed/log/log.h"
#include "zed/net/executor.h"

#include <fcntl.h>
#include <sys/epoll.h>

namespace zed {

namespace net {

    FdEvent::FdEvent(Executor* executor, int fd) : m_executor(executor), m_fd(fd) { }

    FdEvent::FdEvent(int fd) : m_fd(fd) { }

    void FdEvent::addEvents(int event)
    {
        if (m_events & event) {
            LOG_DEBUG << "already has the events";
            return;
        }
        m_events |= event;
        // LOG_DEBUG << "FdEvent:: addEvents";
        update();
    }

    FdEvent::~FdEvent()
    {
        // Fd被一个单例对象FdManager管理，FdManager只有在程序结束的时候才析构。
        // 也就是说FdManager的生命周期长与executor。

        // remove();
        // ::close(m_fd);
    }

    void FdEvent::delEvents(int event)
    {
        if (m_events & event) {
            m_events &= ~event;
            update();
            return;
        }
        LOG_DEBUG << "the event do not exist";
    }

    void FdEvent::remove(bool from_close)
    {
        m_executor->delEvent(m_fd);
        m_events = 0;
        m_executor = nullptr;
        m_read_callback = nullptr;
        m_write_callback = nullptr;
        if (from_close) {
            m_is_nonblock = false;
        }
        LOG_DEBUG << "remove fd [ " << m_fd << " ] from executor";
    };

    void FdEvent::handleEvent(int revents)
    {
        if (!(revents & EPOLLIN) && !(revents && EPOLLOUT)) [[unlikely]] {
            LOG_ERROR << "socket = " << m_fd << "have noknow event" << revents;
            remove();
        }
        if (revents & EPOLLIN) {
            if (m_read_callback) [[likely]] {
                m_read_callback();
            }
        }
        if (revents & EPOLLOUT) {
            if (m_write_callback) [[likely]] {
                m_write_callback();
            }
        }
    }

    void FdEvent::update()
    {
        epoll_event event;
        event.events = m_events;
        event.data.ptr = this;
        m_executor->updateEvent(m_fd, event);
    }

    void FdEvent::setNonBlock() noexcept
    {
        if (m_fd == -1) [[unlikely]] {
            LOG_ERROR << "m_fd == -1";
            return;
        }
        if (m_is_nonblock) [[likely]] {
            LOG_DEBUG << "fd:" << m_fd << " already set nonblock";
            return;
        }
        int flag = ::fcntl(m_fd, F_GETFL, 0);
        ::fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
        flag = ::fcntl(m_fd, F_GETFL, 0);
        if (flag & O_NONBLOCK) {
            m_is_nonblock = true;
            LOG_DEBUG << "set o_nonblock succeeded fd [ " << m_fd << " ]";
        } else {
            LOG_ERROR << "set o_nonblock failed fd [ " << m_fd << " ]";
        }
    }

    namespace detail {

        FdManagerImpl::FdManagerImpl(int size) : m_fds(size)
        {
            for (std::size_t i {0}; i < size; ++i) {
                m_fds[i].reset(new FdEvent(i));
            }
        }

        FdEvent* FdManagerImpl::getFdEvent(int fd)
        {
            {
                std::shared_lock read_lock(m_mutex);
                if (fd < static_cast<int>(m_fds.size())) {
                    FdEvent* ptr = m_fds[fd].get();
                    return ptr;
                }
            }
            std::lock_guard write_lock(m_mutex);

            std::size_t new_size = static_cast<std::size_t>(fd * 1.5);
            for (std::size_t i {m_fds.size()}; i < new_size; ++i) {
                m_fds.emplace_back(new FdEvent(i));
            }
            FdEvent* ptr = m_fds[fd].get();
            return ptr;
        }

    } // namespace detail

} // namespace net

} // namespace zed
