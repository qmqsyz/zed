#ifndef ZED_COMM_THREAD_H_
#define ZED_COMM_THREAD_H_

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "zed/util/noncopyable.h"

namespace zed {

class Thread;

class Thread {
public:
    using Ptr = std::shared_ptr<Thread>;

    explicit Thread(std::function<void()>, const std::string& name = std::string {});

    Thread() noexcept = default;

    ~Thread();

    // ban copy and move
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    pid_t getId() const { return m_tid; }

    const std::string& getName() const { return m_name; }

    void join();

public:
    static pid_t GetCurrentThreadId();

    static Thread* GetCurrentThread();

    static const std::string& GetCurrentThreadName();

private:
    void setDefaultName();

private:
    pid_t                 m_tid {-1};
    std::thread           m_thread {};
    std::string           m_name {};
    std::function<void()> m_cb {};

    static std::atomic<int32_t> s_thread_num;
};

} // namespace zed

#endif // ZED_COMM_THREAD_H_