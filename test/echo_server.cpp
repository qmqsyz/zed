#include "zed/net/acceptor.h"
#include "zed/net/address.h"
#include "zed/net/executor_pool.h"
#include "zed/net/socket.h"

#include <iostream>

using namespace zed;
using namespace zed::coroutine;
using namespace zed::net;

Task<void> Session(int fd)
{
    Socket so(fd);
    char   buf[1024] {0};
    while (true) {
        int n = co_await so.recv(buf, sizeof(buf));
        if (n == 0) {
            break;
        }
        co_await so.send(buf, n);
    }
}

Task<void> Server(int thread_num, const Address::Ptr& addr)
{
    Acceptor     ac(addr);
    ExecutorPool pool(thread_num);
    pool.start();
    while (true) {
        int fd = co_await ac.accept();
        pool.getExecutor()->addTask(Session(fd));
    }
}

int main(int argc, char** argv)
{
    int port = 0;
    if (argc == 1) {
        std::cout << "usage: echo_server thread_num ipv4address [port] \n";
        return 1;
    }
    if (argc == 4) {
        port = atoi(argv[3]);
    }
    auto               addr = zed::net::IPv4Address::Create(argv[2], port);
    zed::net::Executor ex;
    ex.setReactorType(Main);
    ex.addTask(Server(atoi(argv[1]), addr));
    ex.start();
}