#include "zed/coroutine/task.hpp"
#include "zed/log/log.h"
#include "zed/net/executor.h"
#include "zed/net/fd_event.h"
#include "zed/net/timer.h"
#include <vector>

#include <atomic>
#include <iostream>

using namespace zed;
using namespace coroutine;

std::coroutine_handle<> ghandle;

std::atomic<int> g_index {0};

struct TMP {
    constexpr bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle)
    {
        net::FdEvent* p = new net::FdEvent;
        p->setHandle(handle);
        net::TaskManager::GetInstance().push(p);
    }

    void await_resume() { }
};

Task<int> count_lines()
{
    co_await TMP {};
    co_return g_index.fetch_add(1);
}

Task<> usage_example()
{
    int lineCount = co_await count_lines();

    std::cout << "line count = " << lineCount << std::endl;
}

// TODO 完成删除初始TASK

int main()
{
    auto p = std::make_shared<StdoutLogAppender>();
    LoggerManager::GetInstance().addAppender(p);
    LoggerManager::GetInstance().SetLevel(LogLevel::DEBUG);

    net::Executor executor;

    net::TimerEvent::Ptr done(new net::TimerEvent(5, false, [&]() { executor.stop(); }));
    executor.getTimer()->addTimerEvent(done);
    executor.start();

    return 0;
}
