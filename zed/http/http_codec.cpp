#include "zed/http/http_codec.h"
#include "zed/log/log.h"
#include "zed/util/string_util.hpp"

#include <sstream>

namespace zed {

namespace http {

    void HttpCodeC::encode(net::TcpBuffer& output_buffer, HttpResponse& response)
    {
        std::stringstream ss;
        ss << response.m_response_version << " " << response.m_response_code << " "
           << response.m_response_info << "\r\n"
           << detail::EncodeHttpHeader(response.m_response_header) << "\r\n"
           << response.m_response_body << "\r\n";
        std::string str = std::move(ss.str());
        output_buffer.append(str.data(), str.size());
        response.encode_succ = true;
    }

    void HttpCodeC::decode(net::TcpBuffer& buffer, HttpRequest& request)
    {
        std::string_view strs = buffer.toStringView();

        bool is_parse_request_line = false;
        bool is_parse_request_header = false;
        bool is_parse_request_content = false;

        std::size_t read_size = 0;
        std::size_t len = strs.size();

        std::string_view tmp {strs};

        while (true) {
            if (is_parse_request_line == false) {
                size_t i = tmp.find(detail::g_CRLF);
                if (i == tmp.npos) {
                    LOG_DEBUG << "not found CRLF";
                    return;
                }
                if (i == tmp.length() - 2) {
                    LOG_DEBUG << "need to read more data";
                    return;
                }
                is_parse_request_line = parseHttpRequestLine(request, tmp.substr(0, i));
                if (is_parse_request_line == false) {
                    return;
                }
                tmp = tmp.substr(i + 2, len - 2 - i);
                len = tmp.size();
                read_size += i + 2;
            }
            if (is_parse_request_header == false) {
                std::size_t i = tmp.find(detail::g_CRLF_DOUBLE);
                if (i == tmp.npos) {
                    LOG_DEBUG << "not found CRLF CRLF in buffer";
                    return;
                }
                is_parse_request_header = parseHttpRequestHeader(request, tmp.substr(0, i));
                if (is_parse_request_header == false) {
                    return;
                }
                tmp = tmp.substr(i + 4, len - 4 - i);
                len = tmp.size();
                read_size += i + 4;
            }
            if (is_parse_request_content == false) {
                // if map doesn't have Content-Length use std::stoi will throw exception;
                int context_len = std::atoi(request.m_request_header["Content-Length"].c_str());
                if ((int)strs.size() - read_size < context_len) {
                    LOG_DEBUG << "need to read more data";
                    return;
                }
                if (request.m_request_method == POST && context_len != 0) {
                    is_parse_request_content
                        = parseHttpRequestContent(request, tmp.substr(0, context_len));
                    if (is_parse_request_content == false) {
                        return;
                    }
                    read_size += context_len;
                } else {
                    is_parse_request_content = true;
                }
            }
            if (is_parse_request_line && is_parse_request_header && is_parse_request_content) {
                LOG_DEBUG << "parse http request success read size:" << read_size;
                buffer.retrieveAll();
                break;
            }
        }
        request.decode_succ = true;
    }

    bool HttpCodeC::parseHttpRequestLine(HttpRequest& request, const std::string_view& tmp)
    {
        std::size_t first_space_index = tmp.find_first_of(" ");
        std::size_t last_space_index = tmp.find_last_of(" ");
        if (first_space_index == tmp.npos || last_space_index == tmp.npos
            || last_space_index == first_space_index) {
            LOG_DEBUG << "the request line error ,space is not 2";
            return false;
        }

        std::string method {tmp.substr(0, first_space_index)};
        std::transform(method.begin(), method.end(), method.begin(), toupper);
        if (method == "GET") {
            request.m_request_method = HttpMethod::GET;
        } else if (method == "POST") {
            request.m_request_method = HttpMethod::POST;
        } else {
            LOG_DEBUG << "parse http request request line error, not support http method:"
                      << method;
            return false;
        }

        std::string version {tmp.substr(last_space_index + 1, tmp.size() - last_space_index - 1)};
        std::transform(method.begin(), method.end(), method.begin(), toupper);
        if (version != "HTTP/1.1" && version != "HTTP/1.0") {
            LOG_DEBUG << "parse http request request line error, not support http version:"
                      << version;
            return false;
        }
        request.m_request_version = std::move(version);

        std::string url {
            tmp.substr(first_space_index + 1, last_space_index - first_space_index - 1)};
        std::size_t j = url.find("://");
        if (j != url.npos && j + 3 >= url.size()) {
            LOG_ERROR << "parse http request request line error, bad url:" << url;
            return false;
        }
        int l = 0;
        if (j == url.npos) {
            LOG_DEBUG << "url only have path, url is" << url;
        } else {
            url = url.substr(j + 3, last_space_index - first_space_index - j - 4);
            LOG_DEBUG << "delete http prefix, url = " << url;
            j = url.find_first_of("/");
            l = url.size();
            if (j == url.npos || j == url.size() - 1) {
                LOG_DEBUG << "http request root path, and query is empty";
                return true;
            }
            url = url.substr(j + 1, l - j - 1);
        }
        l = url.size();
        j = url.find_first_of("?");
        if (j == url.npos) {
            request.m_request_path = url;
            LOG_DEBUG << "http request path:" << request.m_request_path << "and query is empty";
            return true;
        }
        request.m_request_path = url.substr(0, j);
        request.m_request_query = url.substr(j + 1, l - j - 1);
        LOG_DEBUG << "http request path:" << request.m_request_path
                  << ", and query:" << request.m_request_query;
        util::SplitStrToMap(request.m_request_query, "&", "=", request.m_query_maps);
        return true;
    }

    bool HttpCodeC::parseHttpRequestHeader(HttpRequest& request, const std::string_view& tmp)
    {
        if (tmp.empty() || tmp.size() < 4 || tmp == "\r\n\r\n") {
            return true;
        }
        util::SplitStrToMap(tmp, "\r\n", ":", request.m_request_header);
        return true;
    }

    bool HttpCodeC::parseHttpRequestContent(HttpRequest& request, const std::string_view& tmp)
    {
        if (tmp.empty()) {
            return true;
        }
        request.m_request_body = std::string(tmp);
        return true;
    }

} // namespace http

} // namespace zed
