#include "zed/thread.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "tests/test_util.h"

using namespace zed;

std::atomic<int> num { 0 };

void f(int n)
{
    for (int i = 0; i < n; ++i) {
        std::cout << "Tpoint:" << current_thread::GetThread() << '\t'
                  << "Tid:" << current_thread::GetTid() << '\t'
                  << "Tname:" << current_thread::GetName() << '\n';
        ++num;
    }
}

void test(int n, int m)
{

    std::vector<Thread> vec_thread;
    for (int i = 0; i < n; ++i) {
        vec_thread.emplace_back(std::bind(f,m));
    }
    for (auto& it : vec_thread) {
        it.join();
    }
    std::cout << num << std::endl;
}

int main()
{
    std::cout << "main" << current_thread::GetThread() << ' ' << current_thread::GetTid() << " " << current_thread::GetName() << '\n';
    CalTime(test, 5, 10);
    CalTime(test, 2, 10);
    
    
}

