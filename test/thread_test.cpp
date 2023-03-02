#include "zed/comm/thread.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "test/time_test.h"

using namespace zed;

std::atomic<int> num {0};

void f(int n)
{
    for (int i = 0; i < n; ++i) {
        std::cout << "Tpoint: " << Thread::GetCurrentThread() << '\t'
                  << "Tid: " << Thread::GetCurrentThreadId() << '\t'
                  << "Tname: " << Thread::GetCurrentThreadName() << '\n';
        ++num;
    }
}

void test(int n, int m)
{

    std::vector<Thread::Ptr> vec_thread;
    for (int i = 0; i < n; ++i) {
        vec_thread.emplace_back(new Thread(std::bind(f, m)));
    }
    for (auto& it : vec_thread) {
        it->join();
    }
    std::cout << num << std::endl;
}

int main()
{
    std::cout << "main" << Thread::GetCurrentThread() << ' ' << Thread::GetCurrentThreadId() << " "
              << Thread::GetCurrentThreadName() << '\n';
    SpendTime(test, 5, 10);
    SpendTime(test, 2, 10);
}
