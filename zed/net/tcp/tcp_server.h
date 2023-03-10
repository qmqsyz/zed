// #ifndef ZED_NET_TCPSERVER_H_
// #define ZED_NET_TCPSERVER_H_

// #include "zed/net/acceptor.h"
// #include "zed/net/address.h"
// #include "zed/net/executor_pool.h"

// #include <functional>

// namespace zed {

// namespace net {

//     class TcpServer {
//     public:
//         TcpServer(const Address::Ptr& addr, int worker_num);

//         ~TcpServer();

//         void start();

//         // void setReadBack();

//         // void setWriteBack();

//     private:
//         coroutine::Task<void> handleClient(int fd);

//         coroutine::Task<void> doAccept();

//     private:
//         std::function<std::coroutine_handle<>(Socket, TcpBuffer)> workthread;

//         Acceptor     m_acceptor;
//         ExecutorPool m_sub_executors;
//         Executor     m_main_executor;
//     };

// } // namespace net

// } // namespace zed

// #endif // ZED_NET_TCPSERVER_H_