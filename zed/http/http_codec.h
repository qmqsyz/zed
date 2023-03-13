#ifndef ZED_NET_HTTP_HTTPCODEC_H_
#define ZED_NET_HTTP_HTTPCODEC_H_

#include "zed/http/http_define.h"
#include "zed/net/tcp_buffer.h"

namespace zed {

namespace http {

    class HttpCodeC {
    public:
        HttpCodeC() = default;

        void encode(net::TcpBuffer& input_buffer, HttpResponse& response);

        void decode(net::TcpBuffer& output_buffer, HttpRequest& request);

    private:
        bool parseHttpRequestLine(HttpRequest& requset, const std::string_view& tmp);

        bool parseHttpRequestHeader(HttpRequest& requset, const std::string_view& tmp);

        bool parseHttpRequestContent(HttpRequest& requset, const std::string_view& tmp);
    };

} // namespace http

} // namespace zed

#endif // ZED_NET_HTTP_HTTPCODEC_H_
