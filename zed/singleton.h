#ifndef ZED_SINGLETON_H_
#define ZED_SINGLETON_H_

namespace zed {

template <class T>
class Singleton {
public:
    static T *GetInstance() {
        static T obj;
        return &obj;
    }
};

} // namespace zed

#endif // ZED_SINGLETON_H_