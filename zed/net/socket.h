#ifndef ZED_NET_Socket_H_
#define ZED_NET_Socket_H_

#include "zed/net/address.h"
#include "zed/net/asyn_io.h"
#include "zed/util/noncopyable.h"

namespace zed {

namespace net {

    class Socket : util::Noncopyable {
    public:
        explicit Socket(int sockfd) noexcept;

        ~Socket() noexcept;

        Socket(Socket&& other) noexcept : m_sockfd(other.m_sockfd) { other.m_sockfd = -1; };

        Socket& operator=(Socket&& other) noexcept
        {
            if (this == std::addressof(other)) {
                return *this;
            }
            m_sockfd = other.m_sockfd;
            other.m_sockfd = -1;
            return *this;
        }

        Socket& bind(const Address::Ptr& addr);

        Socket& listen(int flag = SOMAXCONN);

        Socket& setTcpNoDelay(bool on);

        Socket& setReuseAddr(bool on);

        Socket& setReusePort(bool on);

        Socket& setKeepAlive(bool on);

        [[nodiscard]] int getFd() const { return m_sockfd; }

        [[CO_AWAIT_HINT]] auto connect(Address::Ptr addr) const noexcept
        {
            return asyn::Connect(m_sockfd, addr->getAddr(), addr->getAddrLen());
        }

        [[CO_AWAIT_HINT]] auto recv(char* buf, int count, int flags = 0) const noexcept
        {
            return asyn::Recv(m_sockfd, buf, count, flags);
        }

        [[CO_AWAIT_HINT]] auto send(char* buf, int count, int flags = 0) const noexcept
        {
            return asyn::Send(m_sockfd, buf, count, flags);
        }

        int close() noexcept
        {
            const int tmp {m_sockfd};
            m_sockfd = -1;
            FdManager::GetInstance().getFdEvent(tmp)->remove();
            return ::close(tmp);
        }

        int shutdownWrite() const noexcept { return ::shutdown(m_sockfd, SHUT_WR); }

        [[nodiscard]] Address::Ptr getLocalAddr() const;

        [[nodiscard]] Address::Ptr getPeerAddr() const;

    public:
        [[nodiscard]] static Socket CreateTCP(sa_family_t family);

        [[nodiscard]] static Socket CreateUDP(sa_family_t family);

    private:
        int m_sockfd {-1};
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_Socket_H_