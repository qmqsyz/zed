#include "zed/net/fd_event.h"
#include "zed/log/log.h"
#include "zed/net/executor.h"

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
        LOG_DEBUG << "FdEvent:: addEvents";
        update();
    }

    FdEvent::~FdEvent()
    {
        // NOTICE:  FdEvent don't possess the fd, it only package fd and event.
        //          FdEvent exists longer than executor

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
        LOG_DEBUG << "the event no exist";
    }

    void FdEvent::remove()
    {
        m_executor->delEvent(m_fd);
        m_events = 0;
        m_executor = nullptr;
        m_read_callback = nullptr;
        m_write_callback = nullptr;
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
