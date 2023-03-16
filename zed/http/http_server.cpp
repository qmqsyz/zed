#include "zed/http/http_server.h"

#include <functional>

namespace zed {

namespace http {

    HttpServer::HttpServer(std::size_t thread_num) : TcpServer(thread_num)
    {
        using namespace std::placeholders;
        setMessageCallback(std::bind(&HttpServer::messageCallback, this, _1, _2, _3));
    }

    void HttpServer::registerServlet(const std::string& path, const HttpServlet::Ptr& servlet)
    {
        m_dispatcher.registerServlet(path, servlet);
    }

    void HttpServer::messageCallback(net::TcpBuffer& input_buffer,
                                     net::TcpBuffer& output_buffer,
                                     bool&           close_flag)
    {
        m_dispatcher.dispatch(input_buffer, output_buffer, close_flag);
    }

} // namespace http

} // namespace zed
