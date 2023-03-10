// #include "zed/net/tcp_server.h"
// #include "zed/log/log.h"
// #include "zed/net/socket.h"
// #include "zed/net/tcp_buffer.h"

// namespace zed {

// namespace net {

//     TcpServer::TcpServer(const Address::Ptr& addr, int worker_num)
//         : m_acceptor(addr), m_sub_executors(worker_num)
//     {
//         m_main_executor.setReactorType(Main);
//         m_main_executor.addTask(doAccept());
//     }

//     TcpServer::~TcpServer()
//     {
//         m_main_executor.stop();
//     }

//     void TcpServer::start()
//     {
//         m_sub_executors.start();
//         m_main_executor.start();
//     }

//     coroutine::Task<void> TcpServer::doAccept()
//     {
//         while (int fd = co_await m_acceptor.accept()) {
//             if (fd >= 0) [[likely]] {
//                 auto sub_executor = m_sub_executors.getExecutor();
//                 sub_executor->addTask(handleClient(fd));
//             } else {
//                 LOG_ERROR << "accept failed errinfo:" << strerror(errno);
//             }
//         }
//     }

//     coroutine::Task<void> TcpServer::handleClient(int fd)
//     {
//         Socket    socket(fd);
//         TcpBuffer buffer;
//         while (true) {
//             int n = co_await socket.recv (buffer.beginWrite(), buffer.writableBytes());

//             // TODO do something
//         }
//     }

// } // namespace net

// } // namespace zed
