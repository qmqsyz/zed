#ifndef ZED_COMM_NONCOPYABLE_H_
#define ZED_COMM_NONCOPYABLE_H_

namespace zed {

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace zed

#endif // ZED_COMM_NONCOPYABLE_H_