#ifndef ZED_NET_TCPSERVER_H_
#define ZED_NET_TCPSERVER_H_

#include "zed/net/address.h"

namespace zed {

namespace net {

    class TcpServer {
    public:
        TcpServer(const Address::Ptr& addr);

        ~TcpServer();
    }

} // namespace net

} // namespace zed

#endif // ZED_NET_TCPSERVER_H_