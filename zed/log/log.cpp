#include <chrono>
#include <zed/log/log.h>

#include <zed/thread.h>

namespace zed {

LogLevel::Level Logger::g_level{LogLevel::DEBUG};

static thread_local char formatted_time[64];
static thread_local time_t t_last_second{0};

void Logger::SetLevel(LogLevel::Level level) {
    g_level = level;
}

LogLevel::Level Logger::GetLevel() {
    return g_level;
}

std::string_view LogLevel::Tostring(LogLevel::Level level) {
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

LogLevel::Level LogLevel::Fromstring(std::string &str) {
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
                   const char *file_name,
                   int32_t line,
                   const char *func_name)
    : m_level(level), m_file_name(file_name), m_line(line), m_func_name(func_name) {
    setColor();
    addFormattedTime();
    addLevel();
    addTid();
    addFileInformation();
}

LogEvent::~LogEvent() {
    m_ss << "\n"
         << "\e[0m";
    LoggerManager::Getinstance()->log(std::move(m_ss.str()));
}

void LogEvent::addFormattedTime() {
    timeval tv_time;
    gettimeofday(&tv_time, nullptr);
    auto cur_second = tv_time.tv_sec;
    auto cur_microsecond = tv_time.tv_usec;
    if (cur_second != t_last_second) {
        t_last_second = cur_second;
        struct tm tm_time;
        ::localtime_r(&cur_second, &tm_time);
        const char *format = "%Y-%m-%d %H:%M:%S";
        ::strftime(formatted_time, sizeof(formatted_time), format, &tm_time);
    }
    m_ss << '[' << formatted_time << '.' << cur_microsecond << "]\t";
}

void LogEvent::addFileInformation() {
    if (m_level <= LogLevel::DEBUG) {
        m_ss << '[' << m_file_name << ":" << m_line;
        if (m_level <= LogLevel::DEBUG) {
            m_ss << '-' << m_func_name;
        }
        m_ss << "] ";
    }
}

void LogEvent::setColor() {
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

void Logger::addAppender(LogAppender::Ptr appender) {
    std::lock_guard<std::mutex> lock(m_appenders_mutex);
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::Ptr appender) {
    std::lock_guard<std::mutex> lock(m_appenders_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    std::lock_guard<std::mutex> lock(m_appenders_mutex);
    m_appenders.clear();
}

void Logger::log(std::string msg) {
    std::lock_guard<std::mutex> lock(m_appenders_mutex);
    for (auto &appender : m_appenders) {
        appender->log(msg);
    }
}

void StdoutLogAppender::log(const std::string &msg) {
    ::printf("%s", msg.c_str());
}

FileLogAppender::FileLogAppender(const std::string &base_name,
                                 off_t roll_size,
                                 int flush_interval,
                                 int check_every_n)
    : m_file(new LogFile(base_name, roll_size, flush_interval, check_every_n)),
      m_current_buffer{new Buffer} {
    for (int i = 0; i < 2; ++i) {
        m_empty_buffers.emplace_back(new Buffer);
    }
    m_thread = Thread(std::bind(&FileLogAppender::threadFunc, this), "log_thread");
}

FileLogAppender::~FileLogAppender() {
    if (m_running) {
        stop();
    }
}

void FileLogAppender::log(const std::string &msg) {
    std::lock_guard<std::mutex> lock(m_buffer_mutex);
    if (m_current_buffer->avail() > msg.size()) {
        m_current_buffer->append(msg);
    } else {
        m_full_buffers.push_back(std::move(m_current_buffer));
        if (!m_empty_buffers.empty()) {
            m_current_buffer = std::move(m_empty_buffers.front());
            m_empty_buffers.pop_front();
        } else {
            m_current_buffer.reset(new Buffer);
        }
        m_current_buffer->append(msg);
        m_cond.notify_one();
    }
}

void FileLogAppender::stop() {
    m_running = false;
    m_cond.notify_one();
    m_thread.join();
}

void FileLogAppender::threadFunc() {
    while (m_running) {
        {
            std::unique_lock<std::mutex> lock(m_buffer_mutex);
            if (m_full_buffers.empty()) {
                m_cond.wait_for(lock, std::chrono::seconds(3));
            }
            m_full_buffers.push_back(std::move(m_current_buffer));
            if (!m_empty_buffers.empty()) {
                m_current_buffer = std::move(m_empty_buffers.front());
                m_empty_buffers.pop_front();
            } else {
                m_current_buffer.reset(new Buffer);
            }
        }

        if (m_full_buffers.size() > 25) {
            char buf[256];
            ::snprintf(buf, sizeof(buf), "Dropped log messages  %zd larger buffers\n",
                       m_full_buffers.size() - 2);
            m_full_buffers.resize(2);
        }

        for (auto &buffer : m_full_buffers) {
            m_file->append(buffer->data(), buffer->size());
            buffer->reset();
        }
        if (m_full_buffers.size() > 2) {
            m_full_buffers.resize(2);
        }

        m_file->flush();
        m_empty_buffers.splice(m_empty_buffers.end(), m_full_buffers);
    }
}

} // namespace zed
