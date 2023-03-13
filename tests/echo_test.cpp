#include "zed/coroutine/task.hpp"
#include "zed/log/log.h"
#include "zed/net/address.h"
#include "zed/net/executor.h"
#include "zed/net/socket.h"

using namespace zed;
using namespace zed::net;
using namespace zed::coroutine;

Task<void> handleClient(int fd)
{
    Socket socket(fd);
    char   buf[1024];
    int    n = 0;
    while ((n = co_await socket.recv(buf, sizeof(buf))) > 0) {
        LOG_DEBUG << buf;
        co_await socket.send(buf, n);
    }
}

Task<void> server(Address::Ptr addr)
{
    auto accept = Socket::CreateTCP(addr->getFamily());
    accept.bind(addr);
    accept.listen();
    while (int fd = co_await accept.accept()) {
        Executor::GetCurrentExecutor()->addTask(handleClient(fd));
    }
}

int main()
{
    Address::Ptr  addr = IPv4Address::Create("127.0.0.1", 6666);
    net::Executor ex;
    ex.addTask(server(addr));
    ex.start();
}