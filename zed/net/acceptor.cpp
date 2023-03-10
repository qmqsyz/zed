#include "zed/net/acceptor.h"

namespace zed {

namespace net {

    Acceptor::Acceptor(const Address::Ptr& addr)
        : m_listen_socket(Socket::CreateTCP(addr->getFamily()))
    {
        m_listen_socket.setReuseAddr(true).bind(addr).listen();
    }

} // namespace net

} // namespace zed
