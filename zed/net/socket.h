#ifndef ZED_NET_Socket_H_
#define ZED_NET_Socket_H_

#include "zed/net/address.h"
#include "zed/net/lazy_io.h"

namespace zed {

namespace net {

    class Socket {
    public:
        explicit Socket(int sockfd) noexcept;

        ~Socket() noexcept = default;

        Socket& bind(const Address::Ptr& addr);

        Socket& listen(int flag = SOMAXCONN);

        Socket& setTcpNoDelay(bool on);

        Socket& setReuseAddr(bool on);

        Socket& setReusePort(bool on);

        Socket& setKeepAlive(bool on);

        [[nodiscard]] int getFd() const { return m_sockfd; }

        [[CO_AWAIT_HINT]] auto connect(Address::Ptr addr) const noexcept
        {
            return lazy::Connect(m_sockfd, addr->getAddr(), addr->getAddrLen());
        }

        [[CO_AWAIT_HINT]] auto recv(char* buf, int count, int flags = 0) const noexcept
        {
            return lazy::Recv(m_sockfd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] auto send(char* buf, int count, int flags = 0) const noexcept
        {
            return lazy::Send(m_sockfd, buf, count, flags);
        }

        [[nodiscard]] Address::Ptr getLocalAddr() const;

        [[nodiscard]] Address::Ptr getPeerAddr() const;

    public:
        static Socket CreateTCP(sa_family_t family);

        static Socket CreateUDP(sa_family_t family);

    private:
        int m_sockfd;
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_Socket_H_