#ifndef ZED_UTIL_H_
#define ZED_UTIL_H_

#include <cxxabi.h>
#include <filesystem>
#include <string>
#include <vector>

namespace zed {

template <typename T>
const char *GetTypeName() {
    static const char *s_type_name =
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_type_name;
}

namespace file_util {
std::vector<std::string> GetFilesWithExtension(const std::filesystem::path &path,
                                               const std::string_view &extension);
} // namespace file_util

} // namespace zed

#endif // ZED_UTIL_
