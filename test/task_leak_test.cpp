#include "zed/coroutine/task.hpp"

#include <iostream>

using namespace zed;
using namespace coroutine;

std::coroutine_handle<> g_handle;

struct TMP {
    constexpr bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle)
    {
        std::cout << "suspend" << std::endl;
        g_handle = handle;
    }

    void await_resume() { std::cout << "TMP resume" << std::endl; }
};

Task<int> f3()
{
    co_await TMP {};
    co_return 1;
}

Task<> f2()
{
    int v = co_await f3();
    std::cout << v << std::endl;
}

Task<> f1()
{
    co_await f2();
}

int main()
{
    auto t = f1();
    auto handle = t.getHandle();
    t.detach();
    handle.resume();
    g_handle.resume();
}