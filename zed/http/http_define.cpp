#include "zed/http/http_define.h"

#include <sstream>

namespace zed {

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

std::string HttpHeader::getValue(const std::string& key)
{
    return m_headers[key];
}

void HttpHeader::setKeyValue(const std::string& key, const std::string& value)
{
    m_headers.emplace(key, value);
}

std::size_t HttpHeader::getHeaderTotalLength()
{
    std::size_t len = 0;
    for (const auto& [key, value] : m_headers) {
        len += key.size() + 1 + value.size() + 2;
    }
    return len;
}

std::string HttpHeader::toHttpString()
{
    std::stringstream ss;
    for (const auto& [key, value] : m_headers) {
        ss << key << ':' << value << "\r\n";
    }
    return ss.str();
}

} // namespace zed
