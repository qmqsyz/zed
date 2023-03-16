#include "zed/http/http_dispatcher.h"
#include "zed/http/http_codec.h"

#include "zed/log/log.h"

namespace zed {

namespace http {

    void HttpDispatcher::dispatch(net::TcpBuffer& input_buffer,
                                  net::TcpBuffer& out_buffer,
                                  bool&           close_flag)
    {
        // if one tcp have more than one http request
        while (true) {
            HttpRequest request;
            m_codec.decode(input_buffer, request);

            if (!request.decode_succ) {
                return;
            }

            HttpResponse response;
            std::string  url_path = request.m_request_path;

            if (!url_path.empty()) {
                auto it = m_servlets.find(url_path);
                if (it == m_servlets.end()) {
                    LOG_DEBUG << "don't have the servlets";
                    NotFoundHttpServlet servlet;
                    servlet.setCommParam(request, response);
                    servlet.handle(request, response);
                } else {
                    it->second->setCommParam(request, response);
                    it->second->handle(request, response);
                }
            }

            m_codec.encode(out_buffer, response);
        }
    }

    void HttpDispatcher::registerServlet(const std::string& path, HttpServlet::Ptr servlet)
    {
        m_servlets.emplace(path, servlet);
    }

} // namespace http

} // namespace zed
