// #include "zed/net/tcp_connection.h"
// #include "zed/log/log.h"

// namespace zed {

// namespace net {

//     TcpConnection::TcpConnection(Executor* executor, int sockfd) : m_socket(sockfd)
//     {
//         m_state = State::Connected;
//     }

//     TcpConnection::~TcpConnection() { }

//     coroutine::Task<void> TcpConnection::handle()
//     {
//         while (true) {
//             int ret = co_await m_socket.recv(m_input_buffer.beginWrite(),
//                                              m_input_buffer.writableBytes());
//             if (ret > 0) {
//                 m_input_buffer.hasWritten(ret);
//             } else if (ret <= 0) {
//                 // TODO  CHECK TIMEOUT
//                 LOG_ERROR << "socket recv return " << ret << " sysinfo: " << strerror(errno);
//                 break;
//             }
//             if (m_input_buffer.writableBytes() == 0) {
//                 // TODO MAKE BIG;
//                 continue;
//             }

//             // m_message_callback(this,);

//             while (true) {
//                 if (m_output_buffer.readableBytes() == 0) {
//                     break;
//                 }
//                 ret = co_await m_socket.send(m_output_buffer.beginRead(),
//                                              m_output_buffer.readableBytes());
//                 if (ret <= 0) {
//                     // TODO CHECK TIMEOUT
//                     LOG_ERROR << "socket send return " << ret << " sysinfo: " << strerror(errno);
//                     break;
//                 }
//                 m_output_buffer.hasRead(ret);
//                 if (m_output_buffer.readableBytes() == 0) {
//                     LOG_DEBUG << "send all data";
//                     break;
//                 }
//             }
//         }
//     }

// } // namespace net

// } // namespace zed
