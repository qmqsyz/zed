#ifndef ZED_LOG_LOGBUFFER_H_
#define ZED_LOG_LOGBUFFER_H_

#include <cstring>

#include "zed/noncopyable.h"

namespace zed {

const size_t kSmallBuffer = 4000;
const size_t kLargeBuffer = 4000 * 1000;

template <size_t SIZE>
class LogBuffer : Noncopyable {
public:
    using Ptr = std::shared_ptr<LogBuffer>;

    LogBuffer() : m_cur(m_data) {}

    void append(const std::string &str) {
        if (static_cast<size_t>(avail()) > str.size()) {
            ::memcpy(m_cur, str.data(), str.size());
            m_cur += str.size();
        }
    }

    const char *data() const { return m_data; }
    char *current() { return m_cur; }
    size_t size() const { return m_cur - m_data; }
    int avail() const { return static_cast<int>(m_data + sizeof(m_data) - m_cur); }
    void add(size_t len) { m_cur += len; }

    void reset() { m_cur = m_data; }
    void bzero() { std::memset(m_data, 0, sizeof(m_data)); }

private:
    char m_data[SIZE]{};
    char *m_cur{nullptr};
};

} // namespace zed

#endif // ZED_LOG_LOGBUFFER_H_