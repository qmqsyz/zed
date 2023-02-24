#ifndef ZED_LOG_LOGFILE_H_
#define ZED_LOG_LOGFILE_H_

#include <memory>
#include <string>

#include "zed/noncopyable.h"

namespace zed {
class LogFile : Noncopyable {
public:
    using Ptr = std::shared_ptr<LogFile>;

    LogFile(const std::string &base_name, off_t roll_size, int flush_interval, int check_every_num);
    ~LogFile();

    void append(const char *data, size_t len);
    void flush();
    void rollFile();

private:
    static std::string GetLogFileName(const std::string &base_name);

private:
    const static int kRollPerSeconds{60 * 60 * 24};
    const static int kBufferSize{64 * 1024};

    const std::string m_base_name;
    const off_t m_roll_size;
    const int m_flush_interval;
    const int m_check_every_n;
    int m_count{0};
    off_t m_written{0};
    FILE *m_file{};

    time_t m_last_day{0};
    time_t m_last_roll_time{0};
    time_t m_last_flush_time{0};

    char m_buffer[kBufferSize];
};

} // namespace zed

#endif // ZED_LOG_LOGFILE_H_