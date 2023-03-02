#ifndef ZED_UTIL_SINGLETON_HPP_
#define ZED_UTIL_SINGLETON_HPP_

namespace zed {

namespace util {

    template <class T>
    class Singleton {
    public:
        static T& GetInstance()
        {
            static T obj;
            return obj;
        }
    };

} // namespace util

} // namespace zed

#endif // ZED_UTIL_SINGLETON_HPP_