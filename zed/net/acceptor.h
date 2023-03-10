#ifndef ZED_NET_ACCEPTOR_H_
#define ZED_NET_ACCEPTOR_H_

#include "zed/net/address.h"
#include "zed/net/asyn_io.h"
#include "zed/net/socket.h"
#include "zed/util/noncopyable.h"

namespace zed {

namespace net {

    class Acceptor : util::Noncopyable {
    public:
        explicit Acceptor(const Address::Ptr& addr);
        ~Acceptor() noexcept = default;
        Acceptor(Acceptor&&) noexcept = default;
        Acceptor& operator=(Acceptor&&) noexcept = default;

        [[CO_AWAIT_HINT]] auto accept(int flags = 0)
        {
            return asyn::Accept(m_listen_socket.getFd(), nullptr, nullptr, flags);
        }

        [[nodiscard]] int getFd() const noexcept { return m_listen_socket.getFd(); }

    private:
        Socket m_listen_socket;
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_ACCEPTOR_H_