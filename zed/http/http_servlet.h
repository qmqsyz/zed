#ifndef ZED_HTTP_HTTPSERVLET_H_
#define ZED_HTTP_HTTPSERVLET_H_

#include "zed/http/http_define.h"

#include <memory>

namespace zed {

namespace http {

    class HttpServlet : public std::enable_shared_from_this<HttpServlet> {
    public:
        using Ptr = std::shared_ptr<HttpServlet>;

        HttpServlet() noexcept = default;

        virtual ~HttpServlet() noexcept = default;

        virtual void handle(HttpRequest& req, HttpResponse& res) = 0;

        [[nodiscard]] virtual std::string getServletName() = 0;

        void handleNotFound(HttpRequest& req, HttpResponse& res);

        void setHttpCode(HttpResponse& res, const int code);

        void setHttpContentType(HttpResponse& res, const std::string& content_type);

        void setHttpBody(HttpResponse& res, const std::string& body);

        void setCommParam(HttpRequest& req, HttpResponse& res);
    };

    class NotFoundHttpServlet : public HttpServlet {
    public:
        NotFoundHttpServlet() noexcept = default;

        ~NotFoundHttpServlet() noexcept = default;

        void handle(HttpRequest& req, HttpResponse& res) override;

        [[nodiscard]] std::string getServletName() override;
    };

} // namespace http

} // namespace zed

#endif // ZED_HTTP_HTTPSERVLET_H_