#include "zed/http/http_server.h"
#include "zed/log/log.h"
#include "zed/net/address.h"

#include <iostream>
#include <sstream>

using namespace zed;
using namespace zed::net;
using namespace zed::http;

const char* html = "<html><body><h1>The unseen blade is the deadliest </h1><p>%s</p></body></html>";

class QPSHttpServlet : public HttpServlet {
public:
    QPSHttpServlet() = default;
    ~QPSHttpServlet() = default;

    void handle(HttpRequest& req, HttpResponse& res) override
    {
        setHttpCode(res, HTTP_OK);
        setHttpContentType(res, "text/html;charset=utf-8");

        std::stringstream ss;
        ss << "QPSHttpServlet Echo Success!! Your id is," << req.m_query_maps["id"];
        char buf[512];
        sprintf(buf, html, ss.str().c_str());
        setHttpBody(res, std::string(buf));
        LOG_DEBUG << ss.str();
    }

    std::string getServletName() { return "QPSHttpServlet"; }
};

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::cout << "usage: ./echo_server ip port thread_num\n";
        return 1;
    }
    if (argc >= 5) {
        LoggerManager::GetInstance().SetLevel(LogLevel::ERROR);
    }

    Address::Ptr addr = IPv4Address::Create(argv[1], atoi(argv[2]));

    HttpServer server(atoi(argv[3]));
    server.registerServlet("/qps", HttpServlet::Ptr(new QPSHttpServlet));

    server.bind(addr);
    server.start();
}