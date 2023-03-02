#ifndef ZED_COMM_ENV_H_
#define ZED_COMM_ENV_H_

#include <filesystem>
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

#include "zed/util/singleton.hpp"

namespace zed {
class Environment {
public:
    bool init(int argc, char** argv);

    void        add(const std::string& key, const std::string& value);
    bool        have(const std::string& key);
    void        del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_value = "");

    void addHelp(const std::string& key, const std::string& description);
    void removeHelp(const std::string& key);
    void printHelp();

    const std::filesystem::path& getExe() const { return m_exe; }
    const std::filesystem::path& getCwd() const { return m_cwd; }

    std::string getConfigPath();

    bool        setEnv(const std::string_view& key, const std::string_view& value);
    std::string getEnv(const std::string_view& key, const std::string& default_value = "");

private:
    std::shared_mutex m_rwmutex {}; // @brief read-write lock

    std::map<std::string, std::string>               m_args {};
    std::vector<std::pair<std::string, std::string>> m_helps {};

    std::filesystem::path m_exe {};
    std::filesystem::path m_cwd {};
};

using EnvironmentManager = util::Singleton<Environment>;

} // namespace zed

#endif // ZED_COMM_ENV_H_