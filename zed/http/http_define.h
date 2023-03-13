#ifndef ZED_HTTP_HTTPDEFINE_H_
#define ZED_HTTP_HTTPDEFINE_H_

#include <map>
#include <string>

namespace zed {

namespace http {

    enum HttpMethod {
        GET = 1,
        POST = 2,
    };

    enum HttpCode {
        HTTP_OK = 200,
        HTTP_BADREQUEST = 400,
        HTTP_FORBIDDEN = 403,
        HTTP_NOTFOUND = 404,
        HTTP_INTERNALSERVERERROR = 500,
    };

    struct HttpRequest {

        HttpMethod                         m_request_method;
        std::string                        m_request_path;
        std::string                        m_request_query;
        std::string                        m_request_version;
        std::map<std::string, std::string> m_request_header;
        std::string                        m_request_body;
        std::map<std::string, std::string> m_query_maps;

        bool decode_succ {false};
    };

    struct HttpResponse {

        std::string                        m_response_version;
        int                                m_response_code;
        std::string                        m_response_info;
        std::map<std::string, std::string> m_response_header;
        std::string                        m_response_body;

        bool encode_succ {false};
    };

    namespace detail {

        extern const char* g_CRLF;
        extern const char* g_CRLF_DOUBLE;
        extern const char* content_type_text;
        extern const char* default_html_template;

        const char* HttpCodeToString(const int code);

        std::string EncodeHttpHeader(const std::map<std::string, std::string>& headers);

    } // namespace detail

} // namespace http

} // namespace zed

#endif // ZED_HTTP_HTTPDEFINE_H_