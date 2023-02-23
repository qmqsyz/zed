#ifndef ZED_COMM_THREAD_H_
#define ZED_COMM_THREAD_H_

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "zed/comm/noncopyable.h"

namespace zed {

class Thread;

namespace current_thread
{
    pid_t GetTid();
    Thread* GetThread();
    const std::string& GetName();

} // namespace current_thread


class Thread : Noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    using ptr = std::shared_ptr<Thread>;

    explicit Thread(ThreadFunc cb, const std::string& name = std::string {});
    Thread() noexcept = default;
    ~Thread();

    Thread(Thread&&);
    Thread& operator=(Thread&&);

    pid_t getId() const { return m_tid; }
    const std::string& name() { return m_name; }
    void join();

private:
    void setDefaultName();

private:
    pid_t m_tid { -1 };
    std::thread m_thread {};
    std::string m_name {};
    ThreadFunc m_cb {};

    static std::atomic<int32_t> s_thread_num;
};

} // namespace zed

#endif // ZED_COMM_THREAD_H_