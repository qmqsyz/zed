#ifndef ZED_NET_TCPSERVER_H_
#define ZED_NET_TCPSERVER_H_

#include "zed/net/address.h"

#include <memory>

namespace zed {

namespace net {

    class TcpAcceptor {
    public:
        using Ptr = std::shared_ptr<TcpAcceptor>;
        TcpAcceptor(Address::Ptr addr);
    };

    class TcpServer {
    public:
        explicit TcpServer();
        ~TcpServer();

    private:
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_TCPSERVER_H_