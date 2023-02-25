#include "zed/config.h"

#include <filesystem>

#include "zed/env.h"

namespace zed {

static void ListAllMember(const std::string &prefix,
                          const YAML::Node &node,
                          std::list<std::pair<std::string, const YAML::Node>> &output) {
    if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
        LOG_ERROR << "Config invalid name=" << prefix << " : " << node;
        return;
    }
    output.emplace_back(prefix, node);
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(),
                          it->second, output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node &root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    for (auto &[k, node] : all_nodes) {
        if (k.empty()) {
            continue;
        }
        std::string key = k;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::Ptr var = LookupBase(key);

        if (var) {
            if (node.IsScalar()) {
                var->fromString(node.Scalar());
            } else {
                std::stringstream ss;
                ss << node;
                var->fromString(ss.str());
            }
        }
    }
}

void Config::LoadFromConfigDirectory(const std::string &path, bool force) {
    const auto absolute_path = std::filesystem::absolute(path);
    const auto files = file_util::GetFilesWithExtension(absolute_path, ".yaml");

    for (const auto &file : files) {
        try {
            YAML::Node root = YAML::LoadFile(file);
            LoadFromYaml(root);
            LOG_INFO << "Load config file:" << file << " succeeded";
        } catch (const std::exception &e) {
            LOG_ERROR << "Load config file failed! "
                      << "because: " << e.what();
        }
    }
}

ConfigVarBase::Ptr Config::LookupBase(const std::string &name) {
    std::shared_lock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

void Config::Visit(std::function<void(ConfigVarBase::Ptr)> cb) {
    std::shared_lock lock(GetMutex());
    ConfigVarMap &mp = GetDatas();
    for (auto &[_, value] : mp) {
        cb(value);
    }
}
} // namespace zed