#ifndef ZED_UTIL_NONCOPYABLE_H_
#define ZED_UTIL_NONCOPYABLE_H_

namespace zed {

namespace util {

    class Noncopyable {
    public:
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator=(const Noncopyable&) = delete;

    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
    };

} // namespace util

} // namespace zed

#endif // ZED_UTIL_NONCOPYABLE_H_