#include "zed/net/socket.h"
#include "zed/log/log.h"

#include <assert.h>
#include <netinet/tcp.h>

namespace zed {

namespace net {

    Socket::Socket(int sockfd) noexcept : m_sockfd(sockfd) { }

    Socket::~Socket() noexcept
    {
        auto ptr = FdManager::GetInstance().getFdEvent(m_sockfd)->getExecutor();
        if (ptr != nullptr) {
            this->close();
        }
    }

    Socket& Socket::bind(const Address::Ptr& addr)
    {
        const int res = ::bind(m_sockfd, addr->getAddr(), addr->getAddrLen());
        if (res != 0) [[unlikely]] {
            LOG_ERROR << "bind failed fd:" << m_sockfd << " bind addr:" << addr->toString()
                      << " failed errinfo:" << strerror(errno);
            std::terminate();
        }
        LOG_INFO << "sockfd:" << m_sockfd << " bind " << addr->toString();
        return *this;
    }
    Socket& Socket::listen(int flag)
    {
        const int res = ::listen(m_sockfd, flag);
        if (res != 0) {
            LOG_ERROR << "listen failed errinfo:" << strerror(errno);
            std::terminate();
        }
        return *this;
    }

    Socket& Socket::setTcpNoDelay(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval))) {
            LOG_ERROR << "set tcp no delay failed errinfo:" << strerror(errno);
        }
        return *this;
    }

    Socket& Socket::setReuseAddr(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
            LOG_ERROR << "set reuse address failed errinfo:" << strerror(errno);
        }
        return *this;
    }

    Socket& Socket::setReusePort(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
            LOG_ERROR << "set reuse port failed errinfo:" << strerror(errno);
        }
        return *this;
    }

    Socket& Socket::setKeepAlive(bool on)
    {
        int optval = on ? 1 : 0;
        if (::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval))) {
            LOG_ERROR << "set keep alive failed errinfo:" << strerror(errno);
        }
        return *this;
    }

    [[nodiscard]] Address::Ptr Socket::getLocalAddr() const
    {
        struct sockaddr_storage local_addr;
        socklen_t               addr_len = sizeof(local_addr);
        if (::getsockname(m_sockfd, (sockaddr*)&local_addr, &addr_len)) [[unlikely]] {
            LOG_ERROR << "getsockname failed errinfo:" << strerror(errno);
        }
        if (local_addr.ss_family == AF_INET) {
            return std::make_shared<IPv4Address>(*(sockaddr_in*)&local_addr);
        } else {
            return std::make_shared<IPv6Address>(*(sockaddr_in6*)&local_addr);
        }
    }

    [[nodiscard]] Address::Ptr Socket::getPeerAddr() const
    {
        struct sockaddr_storage peer_addr;
        socklen_t               addr_len = sizeof(peer_addr);
        if (::getpeername(m_sockfd, (sockaddr*)&peer_addr, &addr_len)) [[unlikely]] {
            LOG_ERROR << "getsockname failed errinfo:" << strerror(errno);
        }
        if (peer_addr.ss_family == AF_INET) {
            return std::make_shared<IPv4Address>(*(sockaddr_in*)&peer_addr);
        } else {
            return std::make_shared<IPv6Address>(*(sockaddr_in6*)&peer_addr);
        }
    }

    Socket Socket::CreateTCP(sa_family_t family)
    {
        const int sockfd = ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
        assert(sockfd >= 0);
        return Socket {sockfd};
    }

    Socket Socket::CreateUDP(sa_family_t family)
    {
        const int sockfd = ::socket(family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
        assert(sockfd >= 0);
        return Socket {sockfd};
    }

} // namespace net

} // namespace zed
