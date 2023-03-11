#include "zed/http/http_codec.h"
#include "zed/http/http_dispatcher.h"
#include "zed/http/http_servlet.h"
#include "zed/log/log.h"
#include "zed/net/acceptor.h"
#include "zed/net/address.h"
#include "zed/net/executor_pool.h"
#include "zed/net/socket.h"
#include "zed/net/tcp_buffer.h"

#include <iostream>

using namespace zed;
using namespace zed::coroutine;
using namespace zed::net;

HttpDispatcher g_dispatcher;

Task<void> Session(int fd)
{
    Socket      so(fd);
    TcpBuffer   buffer;
    TcpBuffer   outbuffer;
    HttpCodeC   code;
    HttpRequest res;
    char        buf[1024];

    while (true) {
        int n = co_await so.recv(buf, sizeof(buf));
        if (n == 0) {
            break;
        }
        buffer.append(buf, n);
        code.decode(buffer, res);
        if (res.decode_succ) {
            break;
        }
    }
    LOG_DEBUG << " read finished";
    g_dispatcher.dispatch(res, outbuffer);
    LOG_DEBUG << " dispath ok";

    co_await so.send(outbuffer.peek(), outbuffer.readableBytes());
    outbuffer.retrieveAll();
}

Task<void> Server(int thread_num, const Address::Ptr& addr)
{
    Socket ac = Socket::CreateTCP(addr->getFamily());
    ac.bind(addr);
    ac.listen();
    ac.setReuseAddr(true).setTcpNoDelay(true);

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
    auto addr = zed::net::IPv4Address::Create(argv[2], port);

    zed::net::Executor ex;
    ex.setReactorType(Main);
    ex.addTask(Server(atoi(argv[1]), addr));
    ex.start();
}