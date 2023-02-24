#ifndef ZED_UTIL_H_
#define ZED_UTIL_H_

#include <filesystem>
#include <string>
#include <vector>

namespace zed {

namespace file_util {
void GetFilesWithExtension(const std::filesystem::path &path,
                           const std::string_view &extension,
                           std::vector<std::string> &files);
} // namespace file_util

} // namespace zed

#endif // ZED_UTIL_
