#ifndef ZED_HTTP_HTTPDISPATCHER_H_
#define ZED_HTTP_HTTPDISPATCHER_H_

#include "zed/http/http_codec.h"
#include "zed/http/http_servlet.h"
#include "zed/net/tcp_buffer.h"

#include <any>
#include <map>
#include <string>

namespace zed {

namespace http {

    class HttpDispatcher {
    public:
        HttpDispatcher() = default;

        ~HttpDispatcher() = default;

        void dispatch(net::TcpBuffer& input_buffer, net::TcpBuffer& out_buffer, bool& close_flag);

        void registerServlet(const std::string& path, HttpServlet::Ptr servlet);

    private:
        std::map<std::string, HttpServlet::Ptr> m_servlets;
        HttpCodeC                               m_codec;
    };

} // namespace http

} // namespace zed

#endif // ZED_HTTP_HTTPDISPATCHER_H_