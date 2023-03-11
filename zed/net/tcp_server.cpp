// #include "zed/net/tcp_server.h"
// #include "zed/log/log.h"
// #include "zed/net/socket.h"
// #include "zed/net/tcp_buffer.h"

// namespace zed {

// namespace net {

//     TcpServer::TcpServer(size_t thread_num) : m_executor_pool(thread_num) { }

//     TcpServer::~TcpServer() { }

//     bool TcpServer::bind(Address::Ptr addr)
//     {
//         std::vector<Address::Ptr> addrs {addr};
//         std::vector<Address::Ptr> fails;
//         return bind(addrs, fails);
//     }

//     void TcpServer::start()
//     {
//         if (!m_is_stop) {
//             return;
//         }
//         m_is_stop = false;
//         for (auto& sock : m_listen_socks) {
//             m_accept_executor.addTask(startAccept(sock));
//         }
//         m_executor_pool.start();
//         m_accept_executor.start();
//     }

//     bool TcpServer::bind(const std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>&
//     fails)
//     {
//         for (auto& addr : addrs) {
//             Socket sock = Socket::CreateTCP(addr->getFamily());
//             if (sock.bind(addr) == false) {
//                 fails.emplace_back(addr);
//                 continue;
//             }
//             if (sock.listen() == false) {
//                 fails.emplace_back(addr);
//                 continue;
//             }
//             LOG_INFO << "sockfd=" << sock.getFd() << " bind " << addr->toString() << " success";
//             m_listen_socks.emplace_back(std::move(sock));
//         }
//         if (!fails.empty()) {
//             m_listen_socks.clear();
//             return false;
//         }
//         return true;
//     }

//     coroutine::Task<void> TcpServer::handleClient(int fd) { }

//     coroutine::Task<void> TcpServer::startAccept(Socket& sock)
//     {
//         while (!m_is_stop) {
//             int fd = co_await sock.accept();
//             if (fd >= 0) {
//                 m_executor_pool.getExecutor()->addTask(handleClient(fd));
//             }
//         }
//     }
// } // namespace net

// } // namespace zed
