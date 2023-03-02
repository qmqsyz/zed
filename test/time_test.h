#ifndef ZED_TEST_UTILTIME_H_
#define ZED_TEST_UTILTIME_H_

#include <chrono>
#include <iostream>
#include <thread>

namespace zed {

template <typename F, typename... Args>
void SpendTime(F&& f, Args&&... args)
{
    auto start {std::chrono::steady_clock::now()};
    f(std::forward<Args>(args)...);
    auto end {std::chrono::steady_clock::now()};
    auto distance {end - start};
    using Fseconds = std::chrono::duration<double>;
    std::cout << "Spend:" << std::chrono::duration_cast<Fseconds>(distance) << "\n";
}

} // namespace zed

#endif // ZED_TEST_UTILTIME_H_