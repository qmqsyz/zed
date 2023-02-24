#include "zed/log/log_appender.h"

namespace zed {

void StdoutLogAppender::log(const std::string &msg) {
    ::printf("%s", msg.c_str());
}

FileLogAppender::FileLogAppender(const std::string &base_name,
                                 off_t roll_size,
                                 int flush_interval,
                                 int check_every_n)
    : m_file{new LogFile(base_name, roll_size, flush_interval, check_every_n)},
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