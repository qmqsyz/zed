#ifndef ZED_NET_TCPBUFFER_H_
#define ZED_NET_TCPBUFFER_H_

#include <cassert>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <vector>

namespace zed {

namespace net {

    class TcpBuffer {
        static constexpr size_t k_cheap_prepend {8};
        static constexpr size_t k_initial_size {1024};

    public:
        explicit TcpBuffer(int init_size = k_initial_size)
            : m_buffer(init_size), m_read_index(k_cheap_prepend), m_write_index(k_cheap_prepend) {};

        ~TcpBuffer() = default;

        void swap(TcpBuffer& rhs)
        {
            m_buffer.swap(rhs.m_buffer);
            std::swap(m_read_index, rhs.m_read_index);
            std::swap(m_write_index, rhs.m_write_index);
        }

        [[nodiscard]] size_t readableBytes() const noexcept { return m_write_index - m_read_index; }

        [[nodiscard]] size_t writableBytes() const noexcept
        {
            return m_buffer.size() - m_write_index;
        }

        void resize(std::size_t size) { m_buffer.resize(size); }

        [[nodiscard]] std::size_t size() const noexcept { return m_buffer.size(); }

        [[nodiscard]] const char* beginRead() const noexcept { return begin() + m_read_index; }

        [[nodiscard]] char* beginWrite() noexcept { return begin() + m_write_index; }

        [[nodiscard]] size_t prependableBytes() const noexcept { return m_read_index; }

        void hasWritten(size_t len) noexcept
        {
            assert(len <= writableBytes());
            m_write_index += len;
        }

        void hasRead(size_t len) noexcept
        {
            assert(len <= readableBytes());

            if (len < readableBytes()) {
                m_read_index += len;
            } else {
                retrieveAll();
            }
        }

        void unwrite(size_t len) noexcept
        {
            assert(len <= readableBytes());
            m_write_index -= len;
        }

        void retrieveAll() noexcept { m_read_index = m_write_index = k_cheap_prepend; }

        [[nodiscard]] std::string retrieveToString() { return retrieveToString(readableBytes()); }

        [[nodiscard]] std::string retrieveToString(size_t len)
        {
            assert(len <= readableBytes());
            std::string result(beginRead(), len);
            hasRead(len);
            return result;
        }

        [[nodiscard]] std::string_view toStringView() const noexcept
        {
            return std::string_view(beginRead(), readableBytes());
        }

        void append(const void* data, size_t len)
        {
            ensureWritableBytes(len);
            ::memcpy(beginWrite(), data, len);
            hasWritten(len);
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len) {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

    private:
        char* begin() noexcept { return m_buffer.data(); }

        const char* begin() const noexcept { return m_buffer.data(); }

        void makeSpace(size_t len)
        {
            if (writableBytes() + prependableBytes() < len + k_cheap_prepend) {
                m_buffer.resize(m_write_index + len);
            } else {
                size_t n = readableBytes();
                ::memcpy(begin() + k_cheap_prepend, begin() + m_read_index, n);
                m_read_index = k_cheap_prepend;
                m_write_index = k_cheap_prepend + n;

                assert(n == readableBytes());
            }
        }

    private:
        size_t            m_read_index {0};
        size_t            m_write_index {0};
        std::vector<char> m_buffer {};
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_TCPBUFFER_H_