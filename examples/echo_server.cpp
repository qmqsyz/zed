#include "zed/net/tcp_buffer.h"
#include "zed/net/tcp_server.h"

#include <iostream>

using namespace zed;
using namespace zed::coroutine;
using namespace zed::net;

void echo(TcpBuffer& in, TcpBuffer& out, bool& close_flag)
{
    in.swap(out);
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cout << "usage: ./echo_server ip port thread_num\n";
        return 1;
    }
    Address::Ptr addr = IPv4Address::Create(argv[1], atoi(argv[2]));

    TcpServer server(atoi(argv[3]));
    server.bind(addr);
    server.setMessageCallback(echo);
    server.start();
}