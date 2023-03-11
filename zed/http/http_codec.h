#ifndef ZED_NET_HTTP_HTTPCODEC_H_
#define ZED_NET_HTTP_HTTPCODEC_H_

#include "zed/http/http_struct.h"
#include "zed/net/tcp_buffer.h"

namespace zed {

namespace net {

    class HttpCodeC {
    public:
        HttpCodeC() = default;

        void encode(TcpBuffer& buf, HttpResponse& response);

        void decode(TcpBuffer& buf, HttpRequest& request);

    private:
        bool parseHttpRequestLine(HttpRequest& requset, const std::string_view& tmp);

        bool parseHttpRequestHeader(HttpRequest& requset, const std::string_view& tmp);

        bool parseHttpRequestContent(HttpRequest& requset, const std::string_view& tmp);
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_HTTP_HTTPCODEC_H_
