#include "zed/http/http_dispatcher.h"
#include "zed/http/http_codec.h"

#include "zed/log/log.h"

namespace zed {

namespace net {

    void HttpDispatcher::dispatch(HttpRequest& request, TcpBuffer& outbuffer)
    {
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
        HttpCodeC c;
        c.encode(outbuffer, response);
    }

    void HttpDispatcher::registerServlet(const std::string& path, HttpServlet::Ptr servlet)
    {
        m_servlets.emplace(path, servlet);
    }

} // namespace net

} // namespace zed
