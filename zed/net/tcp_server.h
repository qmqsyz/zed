// #ifndef ZED_NET_TCPSERVER_H_
// #define ZED_NET_TCPSERVER_H_

// #include "zed/net/acceptor.h"
// #include "zed/net/address.h"
// #include "zed/net/executor_pool.h"

// #include <sys/types.h>

// namespace zed {

// namespace net {

//     class TcpServer {
//     public:
//         TcpServer(size_t thread_num);

//         virtual ~TcpServer();

//         void start();

//         bool TcpServer::bind(Address::Ptr addr);

//         bool TcpServer::bind(const std::vector<Address::Ptr>& addrs,
//                              std::vector<Address::Ptr>&       fails);

//         void setRecvTimeout(uint64_t time_out) noexcept { m_recv_timeout = time_out; }

//     protected:
//         virtual coroutine::Task<void> handleClient(int fd);

//         virtual coroutine::Task<void> startAccept(Socket& sock);

//     private:
//         std::string         m_name;
//         std::string         m_type {"tcp"};
//         std::vector<Socket> m_listen_socks;
//         Executor            m_accept_executor;
//         ExecutorPool        m_executor_pool;
//         uint64_t            m_recv_timeout;
//         bool                m_is_stop;
//     };

// } // namespace net

// } // namespace zed

// #endif // ZED_NET_TCPSERVER_H_