#include "zed/coroutine/task.hpp"

#include <iostream>

using namespace zed;
using namespace zed::coroutine;

Task<int> do_2()
{
    co_return 10;
}

Task<void> do_1()
{
    int n = co_await do_2();
    std::cout << n << std::endl;
}

int main()
{

    auto t = do_1();
    auto handle = t.getHandle();
    t.detach();
    handle.resume();
    return 0;
}