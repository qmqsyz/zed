#include "zed/net/tcp_server.h"
#include "zed/log/log.h"
#include "zed/net/socket.h"
#include "zed/net/tcp_buffer.h"

namespace zed {

namespace net {

    TcpServer::TcpServer(std::size_t thread_num) : m_executor_pool(thread_num) { }

    TcpServer::~TcpServer()
    {
        if (!m_is_stop) {
            stop();
        }
    }

    bool TcpServer::bind(const Address::Ptr& addr)
    {
        std::vector<Address::Ptr> addrs {addr};
        std::vector<Address::Ptr> fails;
        return bind(addrs, fails);
    }

    void TcpServer::start()
    {
        if (m_is_stop == false) {
            LOG_DEBUG << "tcp server have started";
            return;
        }
        m_is_stop = false;
        for (auto& sock : m_listen_socks) {
            m_accept_executor.addTask(startAccept(sock));
        }
        m_accept_executor.setReactorType(Main);
        m_executor_pool.start();
        m_accept_executor.start();
    }

    void TcpServer::stop()
    {
        m_is_stop = true;
        m_accept_executor.stop();
    }

    bool TcpServer::bind(const std::vector<Address::Ptr>& addrs, std::vector<Address::Ptr>& fails)
    {
        for (auto& addr : addrs) {
            Socket socket = Socket::CreateTCP(addr->getFamily());
            if (socket.bind(addr) == false) {
                fails.emplace_back(addr);
                continue;
            }
            if (socket.listen() == false) {
                fails.emplace_back(addr);
                continue;
            }
            LOG_INFO << "sockfd=" << socket.getFd() << " bind " << addr->toString() << " success";

            socket.setKeepAlive(true);

            socket.setReuseAddr(true);

            m_listen_socks.emplace_back(std::move(socket));
        }
        if (!fails.empty()) {
            m_listen_socks.clear();
            return false;
        }
        return true;
    }

    coroutine::Task<void> TcpServer::handleClient(int fd)
    {
        Socket    socket(fd);
        TcpBuffer input_buffer;
        TcpBuffer output_buffer;

        while (!m_is_stop) {
            {
                char         extrabuf[65536];
                struct iovec vec[2];

                const auto writable = input_buffer.writableBytes();
                vec[0].iov_base = input_buffer.beginWrite();
                vec[0].iov_len = writable;
                vec[1].iov_base = extrabuf;
                vec[1].iov_len = sizeof(extrabuf);
                const int iovcount = (writable < sizeof(extrabuf)) ? 2 : 1;
                auto      ret = co_await socket.readv(vec, iovcount);

                if (ret <= 0) {
                    break;
                } else if (ret < writable) {
                    input_buffer.hasWritten(ret);
                } else {
                    input_buffer.append(extrabuf, ret - writable);
                }
            }

            m_message_callback(input_buffer, output_buffer);

            while (true) {
                if (output_buffer.readableBytes() == 0) {
                    // LOG_DEBUG<<"no data to send";
                    break;
                }

                auto ret = co_await socket.send(output_buffer.beginRead(),
                                                output_buffer.readableBytes());
                if (ret <= 0) {
                    LOG_ERROR << "socket send return " << ret << " sysinfo: " << strerror(errno);
                    break;
                }
                output_buffer.hasRead(ret);
            }
        }
    }

    coroutine::Task<void> TcpServer::startAccept(Socket& sock)
    {
        while (!m_is_stop) {
            int fd = co_await sock.accept();
            LOG_DEBUG << "accept fd: " << fd;
            if (fd >= 0) {
                m_executor_pool.getExecutor()->addTask(handleClient(fd));
            }
        }
    }

} // namespace net

} // namespace zed
