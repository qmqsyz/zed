#include "zed/http/http_servlet.h"
#include "zed/log/log.h"

namespace zed {

extern const char* default_html_template;
extern const char* content_type_text;

void HttpServlet::handleNotFound(const HttpRequest& req, HttpResponse& res)
{
    LOG_DEBUG << "handle NotFound";
    setHttpCode(res, HTTP_NOTFOUND);
    char buf[512];
    ::sprintf(buf, default_html_template, std::to_string(HTTP_NOTFOUND).c_str(),
              HttpCodeToString(HTTP_NOTFOUND));
    res.m_response_body = std::string(buf);
    res.m_response_header.setKeyValue("Context-type", content_type_text);
    res.m_response_header.setKeyValue("Context-Length", std::to_string(res.m_response_body.size()));
}

void HttpServlet::setHttpCode(HttpResponse& res, const int code)
{
    res.m_response_code = code;
    res.m_response_info = std::string(HttpCodeToString(code));
}

void HttpServlet::setHttpContentType(HttpResponse& res, const std::string& content_type)
{
    res.m_response_header.setKeyValue("Context-Type", content_type);
}

void HttpServlet::setHttpBody(HttpResponse& res, const std::string& body)
{
    res.m_response_body = body;
    res.m_response_header.setKeyValue("Context-Length", std::to_string(res.m_response_body.size()));
}

void HttpServlet::setCommParam(HttpRequest& req, HttpResponse& res)
{
    res.m_response_version = req.m_request_version;
    res.m_response_header.m_headers["Connection"] = req.m_request_header.m_headers["Connection"];
    // res.m_response_header.setKeyValue("Connnection",
    // req.m_request_header.getValue("Connection"));
}

void NotFoundHttpServlet::handle(const HttpRequest& req, HttpResponse& res)
{
    handleNotFound(req, res);
}

std::string NotFoundHttpServlet::getServletName()
{
    return "NotFoundHttpServlet";
}

} // namespace zed
