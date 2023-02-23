#ifndef ZED_LOG_LOGAPPENDER_H_
#define ZED_LOG_LOGAPPENDER_H_

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

#include "zed/comm/thread.h"
#include "zed/log/log_buffer.h"
#include "zed/log/log_file.h"

namespace zed {

class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;

    virtual ~LogAppender() = default;
    virtual void log(const std::string& msg) = 0;

protected:
};

class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;

    void log(const std::string& msg) override;
};

class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;

    FileLogAppender(const std::string& base_name,
        off_t roll_size = 100 * 1000 * 1000,
        int flush_interval = 3,
        int check_every_n = 1024);
    ~FileLogAppender();

    void log(const std::string& msg) override;
    void stop();

private:
    void threadFunc();

private:
    using Buffer = LogBuffer<kLargeBuffer>;
    LogFile m_file;

    Buffer::ptr m_current_buffer {};
    std::list<Buffer::ptr> m_empty_buffers {};
    std::list<Buffer::ptr> m_full_buffers {};
    std::mutex m_buffer_mutex {};
    std::condition_variable m_cond {};
    Thread m_thread {};
    std::atomic<bool> m_running { true };
};

} // namespace zed

#endif // ZED_LOG_LOGAPPENDER_H_