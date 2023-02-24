#include "zed/util.h"

namespace zed {

namespace file_util {

void GetFilesWithExtension(const std::filesystem::path &path,
                           const std::string_view &extension,
                           std::vector<std ::string> &files) {
    if (std::filesystem::is_directory(path)) {
        for (auto &de : std::filesystem::directory_iterator(path)) {
            if (de.is_directory()) {
                GetFilesWithExtension(de.path(), extension, files);
            } else {
                if (de.path().extension() == extension) {
                    files.emplace_back(de.path().filename());
                }
            }
        }
    }
}

} // namespace file_util

} // namespace zed
