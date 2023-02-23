#ifndef ZED_COMM_SINGLETON_H_
#define ZED_COMM_SINGLETON_H_

namespace zed {

template <class T>
class Singleton {
public:
    static T* instance()
    {
        static T obj;
        return &obj;
    }
};

} // namespace zed

#endif // ZED_COMM_SINGLETON_H_