#ifndef ZED_NET_EXECUTORPOOL_H_
#define ZED_NET_EXECUTORPOOL_H_

#include "zed/comm/thread.h"
#include "zed/net/executor.h"
#include "zed/util/noncopyable.h"

#include <mutex>

namespace zed {

namespace net {

    class ExecutorPool : util::Noncopyable {
    public:
        ExecutorPool(int thread_num = 1);

        ~ExecutorPool();

        void start();

        Executor* getExecutor();

        Executor* getExecutor(int index);

    private:
        bool                                   m_is_started {false};
        int                                    m_thread_num {0};
        int                                    m_index {0};
        std::vector<std::unique_ptr<Thread>>   m_threads {};
        std::vector<std::unique_ptr<Executor>> m_executors {};
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_EXECUTORPOOL_H_