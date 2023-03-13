#ifndef ZED_NET_ADDRESS_H_
#define ZED_NET_ADDRESS_H_

#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

namespace zed {

namespace net {

    class Address {
    public:
        using Ptr = std::shared_ptr<Address>;
        static Address::Ptr Create(const sockaddr* addr);

        static bool Lookup(std::vector<Address::Ptr>& result,
                           const std::string&         host,
                           int                        family = AF_INET,
                           int                        type = 0,
                           int                        protocol = 0);

        static Address::Ptr
        LookupAny(const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);

    public:
        virtual ~Address() {};

        virtual int getFamily() const = 0;

        virtual const sockaddr* getAddr() const = 0;

        virtual sockaddr* getAddr() = 0;

        virtual socklen_t getAddrLen() const = 0;

        virtual std::string toString() const = 0;
    };

    class IPAddress : public Address {
    public:
        using Ptr = std::shared_ptr<IPAddress>;

        virtual uint32_t getPort() const = 0;

        virtual void setPort(uint16_t port) = 0;
    };

    class IPv4Address : public IPAddress {
    public:
        using Ptr = std::shared_ptr<IPv4Address>;

        static IPv4Address::Ptr Create(const std::string_view& ip, uint16_t port = 0);

    public:
        IPv4Address(const sockaddr_in& addr);

        IPv4Address(const std::string_view& ip, uint16_t port = 0);

        IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

        constexpr int getFamily() const override { return AF_INET; }

        const sockaddr* getAddr() const override { return (sockaddr*)&m_address; }

        sockaddr* getAddr() override { return (sockaddr*)&m_address; }

        constexpr socklen_t getAddrLen() const override { return sizeof(m_address); };

        std::string toString() const override;

        uint32_t getPort() const override { return ntohs(m_address.sin_port); }

        void setPort(uint16_t port) override { m_address.sin_port = htons(port); };

    private:
        sockaddr_in m_address;
    };

    class IPv6Address : public IPAddress {
    public:
        using Ptr = std::shared_ptr<IPv6Address>;

        static IPv6Address::Ptr Create(const std::string_view& ip, uint16_t port = 0);

    public:
        IPv6Address();

        IPv6Address(const sockaddr_in6& address);

        IPv6Address(const std::string_view& address, uint16_t port = 0);

        constexpr int getFamily() const override { return AF_INET6; }

        const sockaddr* getAddr() const override { return (sockaddr*)&m_address; }

        sockaddr* getAddr() override { return (sockaddr*)&m_address; }

        constexpr socklen_t getAddrLen() const override { return sizeof(m_address); }

        std::string toString() const override;

        uint32_t getPort() const override { return ntohs(m_address.sin6_port); }

        void setPort(uint16_t port) override { m_address.sin6_port = htons(port); };

    private:
        sockaddr_in6 m_address;
    };

    // for ostream
    std::ostream& operator<<(std::ostream& os, const Address& addr);

} // namespace net

} // namespace zed

#endif // ZED_NET_ADDRESS_H_