#include "zed/http/http_define.h"

#include <sstream>

namespace zed {

namespace http {

    namespace detail {

        const char* g_CRLF = "\r\n";
        const char* g_CRLF_DOUBLE = "\r\n\r\n";
        const char* content_type_text = "text/html;charset=utf-8";
        const char* default_html_template = "<html><body><h1>%s</h1><p>%s</p></body></html>";

        const char* HttpCodeToString(const int code)
        {
            switch (code) {
            case HTTP_OK:
                return "OK";

            case HTTP_BADREQUEST:
                return "Bad Request";

            case HTTP_FORBIDDEN:
                return "Forbidden";

            case HTTP_NOTFOUND:
                return "Not Found";

            case HTTP_INTERNALSERVERERROR:
                return "Internal Server Error";

            default:
                return "UnKnown code";
            }
        }

        std::string EncodeHttpHeader(const std::map<std::string, std::string>& headers)
        {
            std::stringstream ss;
            for (const auto& [key, value] : headers) {
                ss << key << ": " << value << "\r\n";
            }
            return ss.str();
        }
    } // namespace detail

} // namespace http

} // namespace zed
