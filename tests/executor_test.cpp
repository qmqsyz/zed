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

std::atomic<int> g_index {10};

struct TMP {
    constexpr bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle)
    {
        net::FdEvent* p = net::FdManager::GetInstance().getFdEvent(g_index.fetch_add(1));
        p->setHandle(std::move(handle));
        net::TaskManager::GetInstance().push(p);
    }

    void await_resume() { }
};

Task<int> count_lines()
{
    co_await TMP {};
    throw std::logic_error("test exception");
    co_return g_index;
}

Task<> usage_example()
{
    throw std::logic_error("test first exception");
    try {
        int lineCount = co_await count_lines();
        std::cout << "line count = " << lineCount << std::endl;
    } catch (const std::exception& ex) {
        LOG_ERROR << ex.what();
    }
}

int main()
{
    auto p = std::make_unique<StdoutLogAppender>();
    LoggerManager::GetInstance().setAppender(std::move(p));
    LoggerManager::GetInstance().SetLevel(LogLevel::DEBUG);

    net::Executor executor;

    net::TimerEvent::Ptr done(new net::TimerEvent(5000, false, [&]() { executor.stop(); }));
    executor.getTimer()->addTimerEvent(done);
    std::thread t([&]() {
        for (int i = 0; i < 1000; ++i) {
            executor.addTask(usage_example());
        };
        LOG_DEBUG << "end";
    });
    executor.start();
    t.join();
    return 0;
}
