#include "zed/net/address.h"
#include "zed/log/log.h"

#include <netdb.h>

namespace zed {

namespace net {

    Address::Ptr Address::Create(const sockaddr* addr)
    {
        if (addr == nullptr) {
            return nullptr;
        }

        Address::Ptr result;
        switch (addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            break;
        }
        return result;
    }

    bool Address::Lookup(std::vector<Address::Ptr>& result,
                         const std::string&         host,
                         int                        family,
                         int                        type,
                         int                        protocol)
    {
        addrinfo hints, *results {nullptr}, *next {nullptr};
        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;

        std::string node;
        const char* service {nullptr};

        int error = ::getaddrinfo(host.c_str(), service, &hints, &results);
        if (error) {
            LOG_DEBUG << "[ " << host << " ," << family << " ," << protocol
                      << " ] errinfo = " << strerror(error);
            return false;
        }

        next = results;
        while (next) {
            result.push_back(Create(next->ai_addr));
            next = next->ai_next;
        }
        freeaddrinfo(results);
        return !result.empty();
    }

    Address::Ptr Address::LookupAny(const std::string& host, int family, int type, int protocol)
    {
        std::vector<Address::Ptr> result;
        if (Lookup(result, host, family, type, protocol)) {
            return result[0];
        }
        return nullptr;
    }

    IPv4Address::Ptr IPv4Address::Create(const std::string_view& ip, uint16_t port)
    {
        return std::make_shared<IPv4Address>(ip, port);
    }

    IPv4Address::IPv4Address(const sockaddr_in& address) : m_address {address} { }

    IPv4Address::IPv4Address(const std::string_view& ip, uint16_t port)
    {
        ::memset(&m_address, 0, sizeof(m_address));
        m_address.sin_family = AF_INET;
        m_address.sin_port = htons(port);
        if (::inet_pton(AF_INET, ip.data(), &m_address.sin_addr) != 1) {
            LOG_ERROR << "inet_pton failed [ " << ip << " ] errinfo : " << strerror(errno);
        }
    }

    IPv4Address::IPv4Address(uint32_t address, uint16_t port)
    {
        ::memset(&m_address, 0, sizeof(m_address));
        m_address.sin_family = AF_INET;
        m_address.sin_addr.s_addr = ::htonl(address);
        m_address.sin_port = htons(port);
    }

    std::string IPv4Address::toString() const
    {
        char buf[64];
        if (::inet_ntop(AF_INET, &m_address.sin_addr, buf, sizeof(buf)) == nullptr) {
            LOG_ERROR << "inet_ntop failed sysinfo = " << strerror(errno);
        }
        return std::string(buf).append(":").append(std::to_string(getPort()));
    }
    IPv6Address::IPv6Address()
    {
        ::memset(&m_address, 0, sizeof(m_address));
        m_address.sin6_family = AF_INET6;
    }

    IPv6Address::Ptr IPv6Address::Create(const std::string_view& ip, uint16_t port)
    {
        return std::make_shared<IPv6Address>(ip, port);
    }

    IPv6Address::IPv6Address(const sockaddr_in6& address) : m_address(address) { }

    IPv6Address::IPv6Address(const std::string_view& ip, uint16_t port)
    {
        ::memset(&m_address, 0, sizeof(m_address));
        m_address.sin6_family = AF_INET6;
        m_address.sin6_port = htons(port);
        if (::inet_pton(AF_INET6, ip.data(), &m_address.sin6_addr) != 1) {
            LOG_ERROR << "inet_pton failed [ " << ip << " ] errinfo : " << strerror(errno);
        }
    }

    std::string IPv6Address::toString() const
    {
        char buf[64];
        if (::inet_ntop(AF_INET6, &m_address.sin6_addr, buf, sizeof(buf)) == nullptr) {
            LOG_ERROR << "inet_ntop failed sysinfo = " << strerror(errno);
        }
        return std::string(buf).append(":").append(std::to_string(getPort()));
    }

    std::ostream& operator<<(std::ostream& os, const Address& addr)
    {
        os << addr.toString();
        return os;
    }

} // namespace net

} // namespace zed
