#ifndef ZED_ENV_H_
#define ZED_ENV_H_

#include <filesystem>
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

#include "zed/singleton.h"

namespace zed {
class Environment {
public:
    bool init(int argc, char **argv);

    void add(const std::string &key, const std::string &value);
    bool has(const std::string &key);
    void erase(const std::string &key);
    std::string get(const std::string &key, const std::string &default_value = "");

    void addHelp(const std::string &key, const std::string &description);
    void removeHelp(const std::string &key);
    void printHelp();

    bool setEnv(const std::string_view &key, const std::string_view &value);
    std::string getEnv(const std::string_view &key, const std::string &default_value = "");

    std::filesystem::path getAbsolutePath(const std::string &path) const;
    std::string getAbsoluteWorkPath(const std::string &path) const;
    std::string getConfigPath();

private:
    std::shared_mutex m_rwmutex{}; // @brief read-write lock

    std::map<std::string, std::string> m_args{};
    std::vector<std::pair<std::string, std::string>> m_helps{};

    std::filesystem::path m_path;
};

using EnvironmentManager = Singleton<Environment>;

} // namespace zed

#endif // ZED_ENV_H_