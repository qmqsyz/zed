#include "zed/coroutine/task.hpp"
#include "zed/log/log.h"
#include "zed/net/acceptor.h"
#include "zed/net/executor.h"

using namespace zed;

coroutine::Task<void> f()
{
    auto          addr = net::IPv4Address::Create("127.0.0.1", 8080);
    net::Acceptor ac(addr);
    char          buf[64] {0};
    while (int fd = co_await ac.accept()) {
        net::Socket socket {fd};
        while (co_await socket.recv(buf, sizeof(buf))) {
            LOG_INFO << buf;
            co_await socket.send(buf, sizeof(buf));
        }
    }
}

int main()
{
    net::Executor ex;
    ex.addTask(f());
    ex.start();
}