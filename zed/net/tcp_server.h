#ifndef ZED_NET_TCPSERVER_H_
#define ZED_NET_TCPSERVER_H_

#include "zed/coroutine/task.hpp"
#include "zed/net/address.h"
#include "zed/net/executor_pool.h"
#include "zed/net/socket.h"
#include "zed/net/tcp_buffer.h"
#include "zed/util/noncopyable.h"

#include <functional>
#include <string>
#include <sys/types.h>

namespace zed {

namespace net {

    class TcpServer : util::Noncopyable {
    public:
        using MessageFunc = std::function<void(TcpBuffer&, TcpBuffer&)>;

        TcpServer(std::size_t thread_num);

        virtual ~TcpServer();

        void start();

        void stop();

        bool bind(const Address::Ptr& addr);

        bool bind(const std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails);

        void setRecvTimeout(int64_t timeout) noexcept { m_recv_timeout = timeout; }

        int64_t getRecvTimeout() const noexcept { return m_recv_timeout; }

        void setName(const std::string&& name) { m_name = name; }

        const std::string& getName() const { return m_name; }

        bool isStop() const { return m_is_stop; }

        void setMessageCallback(MessageFunc cb) noexcept { m_message_callback = std::move(cb); }

    private:
        coroutine::Task<void> handleClient(int fd);

        coroutine::Task<void> startAccept(Socket& sock);

    private:
        std::string         m_name {};
        std::vector<Socket> m_listen_socks {};
        Executor            m_accept_executor {};
        ExecutorPool        m_executor_pool;
        int64_t             m_recv_timeout {0}; /// @brief millisecond
        bool                m_is_stop {true};
        MessageFunc         m_message_callback {nullptr};
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_TCPSERVER_H_