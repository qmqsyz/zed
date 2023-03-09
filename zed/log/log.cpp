#include <chrono>
#include <zed/log/log.h>

#include <zed/comm/thread.h>

namespace zed {

LogLevel::Level Logger::g_level {LogLevel::DEBUG};

static thread_local char   formatted_time[64];
static thread_local time_t t_last_second {0};

void Logger::SetLevel(LogLevel::Level level)
{
    g_level = level;
}

LogLevel::Level Logger::GetLevel()
{
    return g_level;
}

std::string_view LogLevel::Tostring(LogLevel::Level level)
{
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARN:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

LogLevel::Level LogLevel::Fromstring(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return toupper(c); });
    if (str == "DEBUG") {
        return LogLevel::DEBUG;
    } else if (str == "INFO") {
        return LogLevel::INFO;
    } else if (str == "WARN") {
        return LogLevel::WARN;
    } else if (str == "ERROR") {
        return LogLevel::ERROR;
    } else if (str == "FATAL") {
        return LogLevel::FATAL;
    } else {
        return LogLevel::UNKNOWN;
    }
}

LogEvent::LogEvent(LogLevel::Level level,
                   const char*     file_name,
                   int32_t         line,
                   const char*     func_name)
    : m_level(level), m_file_name(file_name), m_line(line), m_func_name(func_name)
{
    setColor();
    addFormattedTime();
    addLevel();
    addTid();
    addFileInformation();
}

LogEvent::~LogEvent()
{
    m_ss << "\n"
         << "\e[0m";
    LoggerManager::GetInstance().log(std::move(m_ss.str()));
}

void LogEvent::addFormattedTime()
{
    timeval tv_time;
    gettimeofday(&tv_time, nullptr);
    auto cur_second = tv_time.tv_sec;
    auto cur_microsecond = tv_time.tv_usec;
    if (cur_second != t_last_second) {
        t_last_second = cur_second;
        struct tm tm_time;
        ::localtime_r(&cur_second, &tm_time);
        const char* format = "%Y-%m-%d %H:%M:%S";
        ::strftime(formatted_time, sizeof(formatted_time), format, &tm_time);
    }
    m_ss << '[' << formatted_time << '.' << cur_microsecond << "]\t";
}

void LogEvent::addFileInformation()
{
    m_ss << '[' << m_file_name << ":" << m_line;
    // if (m_level <= LogLevel::DEBUG) {
    //     m_ss << '-' << m_func_name;
    // }
    m_ss << "]\t";
}

void LogEvent::setColor()
{
    switch (m_level) {
    case LogLevel::DEBUG:
        m_ss << "\e[1;34m";
        break;
    case LogLevel::INFO:
        // m_ss << "\e[1;37m";
        break;
    case LogLevel::WARN:
        m_ss << "\e[1;33m";
        break;
    case LogLevel::ERROR:
        m_ss << "\e[1;31m";
        break;
    case LogLevel::FATAL:
        m_ss << "\e[1;35m";
        break;
    default:
        break;
    }
}

Logger::Logger() : m_appender(new StdoutLogAppender) { }

void Logger::log(std::string&& msg)
{
    m_appender->log(std::move(msg));
}

} // namespace zed
