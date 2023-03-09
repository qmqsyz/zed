#include "zed/log/log.h"
#include "zed/net/executor.h"
#include "zed/net/lazy_io.h"

#include <sys/file.h>
#include <sys/stat.h>

using namespace zed;
using namespace coroutine;
using namespace net;

int fd[2];

Task<void> Producer()
{
    char str[64] = "1234567890";
    while (int n = co_await lazy::Write(fd[1], str, strlen(str))) {
        LOG_DEBUG << "write " << n;
    }
}

Task<void> Consumer()
{
    char buf[64];
    ::memset(buf, 0, sizeof(buf));
    while (int n = co_await lazy::Read(fd[0], buf, sizeof(buf))) {
        LOG_DEBUG << "read " << n << " str: " << buf;
    }
}

int main()
{
    pipe(fd);
    LOG_DEBUG << "fd = " << fd;
    Executor ex;
    ex.addTask(Consumer(), false);
    std::thread t([]() {
        Executor ex;
        ex.addTask(Producer(), false);
        ex.start();
    });
    ex.start();
    t.join();
}