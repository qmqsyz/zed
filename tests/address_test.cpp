// copy by sylar

#include "zed/log/log.h"
#include "zed/net/address.h"

#include <map>
#include <vector>

void test()
{
    std::vector<zed::net::Address::Ptr> addrs;

    LOG_INFO << "begin";
    bool v = zed::net::Address::Lookup(addrs, "www.baidu.com");
    LOG_INFO << "end";
    if (!v) {
        LOG_INFO << "lookup fail";
        return;
    }
    LOG_DEBUG << "addrs size = " << addrs.size();
    for (size_t i = 0; i < addrs.size(); ++i) {
        LOG_INFO << i << " - " << addrs[i]->toString();
    }
    auto t = zed::net::Address::LookupAny("www.qq.com");
    LOG_INFO << t->toString();
}

void test_ipv4()
{
    // auto addr = sylar::IPAddress::Create("www.sylar.top");
    auto addr = zed::net::IPv4Address::Create("128.0.0.1", 12345);
    LOG_INFO << addr->toString();
}

void test_ipv6()
{
    auto addr = zed::net::IPv6Address::Create("2400:da00::dbf:0:100", 12345);
    LOG_INFO << addr->toString();
}

int main(int argc, char** argv)
{
    zed::LoggerManager::GetInstance().setAppender(std::make_unique<zed::StdoutLogAppender>());
    test_ipv4();
    test_ipv6();
    test();

    return 0;
}
