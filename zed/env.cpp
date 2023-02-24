#include "zed/env.h"

#include <cstdio>
#include <unistd.h>

#include "zed/config.h"
#include "zed/log/log.h"

namespace zed {

bool Environment::init(int argc, char **argv) {
    m_path = argv[0];
    // TODO use getopt()

    return true;
}

void Environment::add(const std::string &key, const std::string &value) {
    std::lock_guard<std::shared_mutex> lock(m_rwmutex);
    m_args.emplace(key, value);
}

bool Environment::has(const std::string &key) {
    std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    return m_args.find(key) != m_args.end();
}

void Environment::erase(const std::string &key) {
    std::lock_guard<std::shared_mutex> lock(m_rwmutex);
    m_args.erase(key);
}

std::string Environment::get(const std::string &key, const std::string &default_value) {
    std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    auto it = m_args.find(key);
    return it != m_args.end() ? it->second : default_value;
}

void Environment::addHelp(const std::string &key, const std::string &description) {
    removeHelp(key);
    std::lock_guard<std::shared_mutex> lock(m_rwmutex);
    m_helps.emplace_back(key, description);
}

void Environment::removeHelp(const std::string &key) {
    std::lock_guard<std::shared_mutex> lock(m_rwmutex);
    for (auto it = m_helps.begin(); it != m_helps.end(); ++it) {
        if (it->first == key) {
            it = m_helps.erase(it);
        } else {
            ++it;
        }
    }
}

void Environment::printHelp() {
    std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    ::printf("Usage: %s [options]\n", m_path.c_str());
    for (auto &[key, description] : m_helps) {
        ::printf("-%s:%s\n", key.c_str(), description.c_str());
    }
}

bool Environment::setEnv(const std::string_view &key, const std::string_view &value) {
    return !::setenv(key.data(), value.data(), 1);
}

std::string Environment::getEnv(const std::string_view &key, const std::string &default_value) {
    const char *value = ::getenv(key.data());
    if (value == nullptr) {
        return default_value;
    }
    return value;
}

std::filesystem::path Environment::getAbsolutePath(const std::string &path) const {
    return std::filesystem::absolute(path);
}

std::string Environment::getAbsoluteWorkPath(const std::string &path) const {
    if (path.empty()) {
        return "/";
    }
    if (path[0] == '/') {
        return path;
    }
    auto g_server_workpath = Config::Lookup<std::string>("server.work_path");
    return g_server_workpath->getValue() + "/" + path;
}

std::string Environment::getConfigPath() {
    return getAbsolutePath(get("c", "conf"));
}

} // namespace zed
