#include "zed/log/log_file.h"

#include <cstdio>
#include <ctime>

namespace zed {
LogFile::LogFile(const std::string& base_name,
    off_t roll_size,
    int flush_interval,
    int check_every_num)
    : m_base_name { base_name }
    , m_roll_size { roll_size }
    , m_flush_interval { flush_interval }
    , m_check_every_n { check_every_num }
{
    rollFile();
    ::setbuffer(m_file, m_buffer, sizeof(m_buffer));
}
LogFile::~LogFile()
{
    ::fclose(m_file);
}

void LogFile::append(const char* data, size_t len)
{
    size_t written = 0;
    while (written < len) {
        size_t remain = len - written;
        size_t n = ::fwrite_unlocked(data + written, 1, remain, m_file);
        written += n;
    }

    m_written += written;
    if (m_written > m_roll_size) {
        rollFile();
    } else {
        ++m_count;
        if (m_count > m_check_every_n) {
            m_count = 0;
            time_t now = ::time(NULL);
            time_t current_day = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (current_day != m_last_day) { // isn't same day
                rollFile();
            } else if (now - m_last_flush_time > m_flush_interval) { // exceed flush interval
                m_last_flush_time = now;
                flush();
            }
        }
    }
}

void LogFile::flush()
{
    ::fflush(m_file);
}

void LogFile::rollFile()
{
    std::string file_name = GetLogFileName(m_base_name);
    time_t now = ::time(nullptr);
    time_t current_day = now / kRollPerSeconds_ * kRollPerSeconds_;
    if (now > m_last_roll_time) {
        m_last_day = current_day;
        m_last_flush_time = now;
        m_last_roll_time = now;
        m_written = 0;
        m_file = ::fopen(file_name.c_str(), "ae");
    }
}

std::string LogFile::GetLogFileName(const std::string& base_name)
{
    std::string file_name { base_name };
    file_name.reserve(file_name.size() + 64);

    time_t now_time = ::time(nullptr);

    struct tm tm_time;
    ::localtime_r(&now_time, &tm_time);

    char buf[32];
    ::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &tm_time);

    file_name.append(buf);
    file_name.append(".log");
    return file_name;
}

} // namespace zed