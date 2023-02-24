#include <chrono>
#include <iostream>

template <typename F, typename... Args>
void CalTime(F &&f, Args &&...args) {
    auto start{std::chrono::steady_clock::now()};
    f(std::forward<Args>(args)...);
    auto end{std::chrono::steady_clock::now()};
    auto distance{end - start};
    using Fseconds = std::chrono::duration<double>;
    std::cout << "Spend:" << std::chrono::duration_cast<Fseconds>(distance) << "\n";
}