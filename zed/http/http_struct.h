#ifndef ZED_HTTP_HTTPSTRUCT_H_
#define ZED_HTTP_HTTPSTRUCT_H_

#include <zed/http/http_define.h>

#include <map>
#include <memory>
#include <string>

namespace zed {

struct HttpRequest {
    using Ptr = std::shared_ptr<HttpRequest>;

    HttpMethod  m_request_method;
    std::string m_request_path;
    std::string m_request_query;
    std::string m_request_version;
    HttpHeader  m_request_header;
    std::string m_request_body;
    bool        decode_succ {false};
    bool        encode_succ {false};

    std::map<std::string, std::string> m_query_maps;
};

struct HttpResponse {
    using Ptr = std::shared_ptr<HttpResponse>;

    std::string m_response_version;
    int         m_response_code;
    std::string m_response_info;
    HttpHeader  m_response_header;
    std::string m_response_body;
    bool        decode_succ {false};
    bool        encode_succ {false};
};

} // namespace zed

#endif // ZED_HTTP_HTTPSTRUCT_H_