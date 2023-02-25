#include "zed/util.h"

namespace zed {

namespace file_util {

std::vector<std::string> GetFilesWithExtension(const std::filesystem::path &path,
                                               const std::string_view &extension) {
    std::vector<std::string> files;
    if (std::filesystem::is_directory(path)) {
        for (auto &de : std::filesystem::recursive_directory_iterator(path)) {
            if (de.path().extension() == extension) {
                files.emplace_back(de.path().string());
            }
        }
    }
    return files;
}

} // namespace file_util

} // namespace zed
