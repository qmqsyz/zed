#include "zed/comm/thread.h"

#include <future>

namespace zed {

static thread_local pid_t       t_thread_id {0};
static thread_local Thread*     t_thread {nullptr};
static thread_local std::string t_thread_name {};

std::atomic<int32_t> Thread::s_thread_num {0};

pid_t Thread::GetCurrentThreadId()
{
    if (t_thread_id == 0) [[unlikely]] {
        t_thread_id = ::syscall(SYS_gettid);
    }
    return t_thread_id;
}

Thread* Thread::GetCurrentThread()
{
    return t_thread;
}

const std::string& Thread::GetCurrentThreadName()
{
    return t_thread_name;
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb {std::move(cb)}, m_name {name}
{
    if (m_name.empty()) {
        setDefaultName();
    }

    std::promise<void> p;
    m_thread = std::thread([&]() {
        m_tid = GetCurrentThreadId();
        t_thread_name = m_name;
        t_thread = this;
        p.set_value();
        m_cb();
    });
    p.get_future().wait();
}

Thread::~Thread()
{
    if (m_thread.joinable()) {
        m_thread.detach();
    }
}

void Thread::join()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Thread::setDefaultName()
{
    int  num = s_thread_num.fetch_add(1);
    char buf[32];
    ::snprintf(buf, sizeof(buf), "Thread_%d", num);
    m_name = buf;
}

} // namespace zed
