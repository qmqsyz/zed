#ifndef ZED_NET_FDEVENT_H_
#define ZED_NET_FDEVENT_H_

#include <coroutine>
#include <functional>
#include <memory>
#include <shared_mutex>

#include "zed/util/singleton.hpp"

namespace zed {

namespace net {

    class Executor;

    class FdEvent {
    public:
        using Func = std::function<void()>;

        FdEvent(Executor* executor, int fd = -1);

        FdEvent(int fd = -1);

        virtual ~FdEvent();

        void addEvents(int events);

        void delEvents(int events);

        void remove(bool from_close = false);

        void handleEvent(int revents);

        [[nodiscard]] int getFd() const noexcept { return m_fd; }

        void setHandle(std::coroutine_handle<> handle) { m_handle = handle; };

        [[nodiscard]] std::coroutine_handle<> getHandle() const { return m_handle; }

        [[nodiscard]] Executor* getExecutor() const noexcept { return m_executor; }

        void setExecutor(Executor* executor) noexcept { m_executor = executor; }

        void setNonBlock() noexcept;

        [[nodiscard]] bool isNonBlock() const noexcept { return m_is_nonblock; };

        void setReadCallback(Func cb) { m_read_callback = std::move(cb); }

        void setWriteCallback(Func cb) { m_write_callback = std::move(cb); }

    private:
        void update();

    protected:
        int                     m_fd {-1};
        int                     m_events {0};
        bool                    m_is_nonblock {false};
        Executor*               m_executor {nullptr};
        std::coroutine_handle<> m_handle {nullptr};
        Func                    m_read_callback {};
        Func                    m_write_callback {};
    };

    namespace detail {

        class FdManagerImpl {
        public:
            FdManagerImpl(int size = 1000);

            FdEvent* getFdEvent(int fd);

        private:
            std::shared_mutex                     m_mutex {};
            std::vector<std::unique_ptr<FdEvent>> m_fds {};
        };

    } // namespace detail

    using FdManager = util::Singleton<detail::FdManagerImpl>;

} // namespace net

} // namespace zed

#endif // ZED_NET_FDEVENT_H_