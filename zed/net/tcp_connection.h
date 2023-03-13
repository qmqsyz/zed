// #ifndef ZED_NET_TCPCONNECTION_H_
// #define ZED_NET_TCPCONNECTION_H_

// #include "zed/net/executor.h"
// #include "zed/net/socket.h"
// #include "zed/net/tcp_buffer.h"
// #include "zed/util/noncopyable.h"

// #include <memory>

// namespace zed {

// namespace net {

//     class TcpConnection : public std::enable_shared_from_this<TcpConnection>, util::Noncopyable {
//         enum class State { Disconnected, Connecting, Connected, Disconnecting };

//     public:
//         TcpConnection(Executor* executor, int sockfd);

//         ~TcpConnection();

//         [[nodiscard]] TcpBuffer& getInputBuffer() { return m_input_buffer; }

//         [[nodiscard]] TcpBuffer& getOutBuffer() { return m_output_buffer; }

//         void connectEstablished();

//         void conenctDestroyed();

//         void send(std::string_view s);

//     private:
//         coroutine::Task<void> handle();

//     private:
//         Executor* m_executor;
//         Socket    m_socket;
//         TcpBuffer m_input_buffer;
//         TcpBuffer m_output_buffer;

//         State m_state;
//     };

// } // namespace net

// } // namespace zed

// #endif // ZED_NET_TCPCONNECTION_H_
