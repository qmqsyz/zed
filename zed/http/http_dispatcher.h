#ifndef ZED_NET_HTTP_HTTPDISPATCHER_H_
#define ZED_NET_HTTP_HTTPDISPATCHER_H_

#include "zed/http/http_servlet.h"
#include "zed/net/tcp_buffer.h"

#include <any>
#include <map>
#include <string>

namespace zed {

namespace net {

    class HttpDispatcher {
    public:
        HttpDispatcher() = default;

        ~HttpDispatcher() = default;

        void dispatch(HttpRequest& request, TcpBuffer& outbuffer);

        void registerServlet(const std::string& path, HttpServlet::Ptr servlet);

    private:
        std::map<std::string, HttpServlet::Ptr> m_servlets;
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_HTTP_HTTPDISPATCHER_H_