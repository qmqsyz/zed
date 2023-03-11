#ifndef ZED_HTTP_HTTPDEFINE_H_
#define ZED_HTTP_HTTPDEFINE_H_

#include <map>
#include <string>

namespace zed {

extern const char* g_CRLF;
extern const char* g_CRLF_DOUBLE;
extern const char* content_type_text;
extern const char* default_html_template;

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

const char* HttpCodeToString(const int code);

struct HttpHeader {
public:
    HttpHeader() = default;

    virtual ~HttpHeader() = default;

    [[nodiscard]] std::string getValue(const std::string& key);

    void setKeyValue(const std::string& key, const std::string& value);

    [[nodiscard]] std::size_t getHeaderTotalLength();

    [[nodiscard]] std::string toHttpString();

    std::map<std::string, std::string> m_headers;
};

} // namespace zed

#endif // ZED_HTTP_HTTPDEFINE_H_