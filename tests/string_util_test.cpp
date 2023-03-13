#include "zed/util/string_util.hpp"

#include <iostream>

int main()
{
    std::string_view str {
        "Connection: keep-alive\r\nCache-Control: max-age=0\r\nsec-ch-ua-mobile: "
        "?0\r\nsec-ch-ua-platform: macOS\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: "
        "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/96.0.4664.110 Safari/537.36\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: "
        "navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, "
        "deflate, br\r\nAccept-Language: zh-CN,zh;q=0.9,en;q=0.8\r\nCookie: "
        "BIDUPSID=8B0207CE0B6364E5934651E84F17999B; PSTM=1619707475;\r\n "};

    std::map<std::string, std::string> mp;
    zed::util::SplitStrToMap(str, "\r\n", ":", mp);
    for (auto& [k, v] : mp) {
        std::cout << k << ":" << v << "\n";
    }
}