#include <iostream>
#include <zed/coroutine/task.hpp>

using namespace zed;
using namespace coro;

Task<int> count_lines()
{
    co_return 1;
}

Task<> usage_example()
{
    int lineCount = co_await count_lines();

    std::cout << "line count = " << lineCount << std::endl;
}

int main()
{
    auto t = usage_example();
    t.getHandle().resume();
}
