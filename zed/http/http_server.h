#ifndef ZED_HTTP_HTTPSERVER_H_
#define ZED_HTTP_HTTPSERVER_H_

#include "zed/http/http_dispatcher.h"
#include "zed/net/tcp_server.h"
#include "zed/util/noncopyable.h"

namespace zed {

namespace http {

    class HttpServer : public net::TcpServer {
    public:
        HttpServer(std::size_t thread_num);

        ~HttpServer() = default;

        void registerServlet(const std::string& path, const HttpServlet::Ptr& servlet);

    private:
        void messageCallback(net::TcpBuffer& input_buffer,
                             net::TcpBuffer& output_buffer,
                             bool&           close_flag);

    private:
        HttpDispatcher m_dispatcher;
    };

} // namespace http

} // namespace zed

#endif // ZED_HTTP_HTTPSERVER_H_