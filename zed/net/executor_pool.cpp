#include "zed/net/executor_pool.h"
#include "zed/log/log.h"

#include <future>

namespace zed {

namespace net {

    ExecutorPool::ExecutorPool(int thread_num) : m_thread_num(thread_num)
    {
        assert(m_thread_num > 0);
    }

    ExecutorPool::~ExecutorPool()
    {
        for (auto& executor : m_executors) {
            executor->stop();
        }
        for (auto& thread : m_threads) {
            thread->join();
        }
    }

    void ExecutorPool::start()
    {
        m_executors.reserve(m_thread_num);
        for (int i {0}; i < m_thread_num; ++i) {
            m_executors.emplace_back(new Executor);
        }

        m_threads.reserve(m_thread_num);
        for (int i {0}; i < m_thread_num; ++i) {
            std::promise<void> p;
            m_threads.emplace_back(new Thread([&]() {
                p.set_value();
                m_executors[i]->start();
            }));
            p.get_future().wait();
        }
    }

    Executor* ExecutorPool::getExecutor()
    {
        if (m_index == m_thread_num) {
            m_index = 0;
        }
        return m_executors[m_index++].get();
    }

    Executor* ExecutorPool::getExecutor(int index)
    {
        if (index >= 0 && index < m_thread_num) [[likely]] {
            return m_executors[index].get();
        }
        LOG_ERROR << "index out range ,current range [ 0 - " << m_thread_num << " ]";
        return nullptr;
    }

} // namespace net

} // namespace zed
