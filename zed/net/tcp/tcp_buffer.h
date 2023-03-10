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

        [[nodisacrd]] size_t prependableBytes() const noexcept { return m_read_index; }

        [[nodiscard]] const char* peek() const noexcept { return begin() + m_read_index; }

        [[nodisacrd]] char* beginWrite() noexcept { return begin() + m_write_index; }

        void hasWritten(size_t len) noexcept
        {
            assert(len <= writableBytes());
            m_write_index += len;
        }

        void unwrite(size_t len) noexcept
        {
            assert(len <= readableBytes());
            m_write_index -= len;
        }

        [[nodiscard]] const char* beginWrite() const noexcept { return begin() + m_write_index; }

        /// @brief 查找\\r\\n
        [[nodiscard]] const char* findCRLF() const noexcept
        {
            const char* crlf
                = static_cast<const char*>(::memmem(peek(), readableBytes(), k_CRLF, 2));
            return crlf == beginWrite() ? nullptr : crlf;
        }

        [[nodiscard]] const char* findCRLF(const char* start) const noexcept
        {
            assert(peek() <= start);
            assert(start <= beginWrite());

            const char* crlf
                = static_cast<const char*>(::memmem(start, readableBytes(), k_CRLF, 2));
            return crlf == beginWrite() ? nullptr : crlf;
        }

        /// @brief 找到结束符
        [[nodiscard]] const char* findEOL() const noexcept
        {
            const void* eol = memchr(peek(), '\n', readableBytes());
            return static_cast<const char*>(eol);
        }

        [[nodiscard]] const char* findEOL(const char* start) const noexcept
        {
            assert(peek() <= start);
            assert(start <= beginWrite());

            const void* eol = memchr(start, '\n', beginWrite() - start);
            return static_cast<const char*>(eol);
        }

        void retrieve(size_t len) noexcept
        {
            assert(len <= readableBytes());

            if (len < readableBytes()) {
                m_read_index += len;
            } else {
                retrieveAll();
            }
        }

        void retrieveAll() noexcept { m_read_index = m_write_index = k_cheap_prepend; }

        [[nodisacrd]] std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        [[nodisacrd]] std::string retrieveAsString(size_t len)
        {
            assert(len <= readableBytes());
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        [[nodisacrd]] std::string_view getStringView() const noexcept
        {
            return std::string_view(peek(), readableBytes());
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
        static const char k_CRLF[];

    private:
        size_t            m_read_index {0};
        size_t            m_write_index {0};
        std::vector<char> m_buffer {};
    };

} // namespace net

} // namespace zed

#endif // ZED_NET_TCPBUFFER_H_