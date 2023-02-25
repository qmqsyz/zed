#include "zed/env.h"

#include <cstdio>
#include <unistd.h>

#include "zed/config.h"
#include "zed/log/log.h"

namespace zed {

bool Environment::init(int argc, char **argv) {
    m_exe = std::filesystem::absolute(argv[0]);
    m_cwd = std::filesystem::current_path();

    char o;
    const char *optstring{"c::d::f::p::"};
    while ((o = ::getopt(argc, argv, optstring)) != -1) {
        std::string arg{optarg == nullptr ? "" : optarg};
        switch (o) {
        case 'c':
            add("c", arg);
            break;
        case 'f':
            add("f", arg);
            break; 
        case 'd':
            add("d", arg);
            break;
        case 'p':
            add("p", arg);
            break;
        case '?':
            LOG_ERROR << "Invalid arg index=" << optind << " arg=" << arg;
            return false;
        }
    }
    return true;
}

void Environment::add(const std::string &key, const std::string &value) {
    std::lock_guard<std::shared_mutex> lock(m_rwmutex);
    m_args.emplace(key, value);
}

bool Environment::have(const std::string &key) {
    std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    return m_args.find(key) != m_args.end();
}

void Environment::del(const std::string &key) {
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
    for (auto it = m_helps.begin(); it != m_helps.end();) {
        if (it->first == key) {
            it = m_helps.erase(it);
        } else {
            ++it;
        }
    }
}

void Environment::printHelp() {
    std::shared_lock<std::shared_mutex> lock(m_rwmutex);
    ::printf("Usage: %s [options]\n", m_exe.filename().c_str());
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

std::string Environment::getConfigPath() {
    return std::filesystem::absolute(get("c", "conf"));
}

} // namespace zed
