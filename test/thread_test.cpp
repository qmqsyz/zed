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
        std::cout << "Tpoint: " << this_thread::GetThread() << '\t'
                  << "Tid: " << this_thread::GetId() << '\t' << "Tname: " << this_thread::GetName()
                  << '\n';
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
    std::cout << "Tpoint: " << this_thread::GetThread() << '\t' << "Tid: " << this_thread::GetId()
              << '\t' << "Tname: " << this_thread::GetName() << '\n';

    SpendTime(test, 5, 10);
    SpendTime(test, 2, 10);
}
