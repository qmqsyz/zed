#ifndef ZED_COMM_CONFIG_H_
#define ZED_COMM_CONFIG_H_

#include <algorithm>
#include <memory>
#include <shared_mutex>

#include "zed/log/log.h"
#include "zed/util/lexical_cast.hpp"
#include "zed/util/utils.h"

namespace zed {

class ConfigVarBase {
public:
    using Ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase(const std::string& name, const std::string& description = "")
        : m_name {name}, m_description {description}
    {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    };

    virtual ~ConfigVarBase() = default;

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string      toString() = 0;
    virtual bool             fromString(const std::string& str) = 0;
    virtual std::string_view getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

template <typename T,
          class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    using Ptr = std::shared_ptr<ConfigVar>;
    using Callback = std::function<void(const T& old_value, const T& new_value)>;

    ConfigVar(const std::string& name, const T& default_value, const std::string description = "")
        : ConfigVarBase(name, description), m_value(default_value)
    {
    }

    std::string toString() override
    {
        try {
            std::lock_guard lock(m_rwmutex);
            return ToStr()(m_value);
        } catch (std::exception& e) {
            LOG_ERROR << "ConfigVar::toString exception " << e.what()
                      << " convert: " << getTypeName() << " to string"
                      << " name=" << m_name;
        }
        return "";
    }

    bool fromString(const std::string& str) override
    {
        try {
            setValue(FromStr()(str));
            return true;
        } catch (std::exception& e) {
            LOG_ERROR << "ConfigVar::fromString exception " << e.what() << " convert: string to "
                      << getTypeName() << " name=" << m_name << " - " << str;
        }
        return false;
    }

    const T getValue()
    {
        std::shared_lock lock(m_rwmutex);
        return m_value;
    }

    void setValue(const T& value)
    {
        {
            std::shared_lock lock(m_rwmutex);
            if (value == m_value) {
                return;
            }
            for (auto& [_, cb] : m_cbs) {
                cb(m_value, value);
            }
        }
        std::lock_guard<std::shared_mutex> lock(m_rwmutex);
        m_value = value;
    }

    std::string_view getTypeName() const override { return GetTypeName<T>(); }

    uint64_t addListener(Callback cb)
    {
        static uint64_t                    s_fun_id = 0;
        std::lock_guard<std::shared_mutex> lock(m_rwmutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    void delListener(uint64_t key)
    {
        std::lock_guard<std::shared_mutex> lock(m_rwmutex);
        m_cbs.erase(key);
    }

    Callback getListener(uint64_t key)
    {
        std::shared_lock lock(m_rwmutex);
        auto             it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener()
    {
        std::lock_guard lock(m_rwmutex);
        m_cbs.clear();
    }

private:
    std::shared_mutex m_rwmutex;
    T                 m_value;
    /// @brief callbacks
    std::map<uint64_t, Callback> m_cbs;
};

class Config {
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::Ptr>;

    template <typename T>
    static ConfigVar<T>::Ptr
    Lookup(const std::string& name, const T& default_value, const std::string& description = "")
    {
        std::lock_guard lock(GetMutex());
        auto            it = GetDatas().find(name);
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp) {
                LOG_INFO << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                LOG_ERROR << "Lookup name=" << name << " exists but type not " << GetTypeName<T>()
                          << "real_type=" << it->second->getTypeName() << " "
                          << it->second->toString();
                return nullptr;
            }
        }

        if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
            LOG_ERROR << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::Ptr value(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = value;
        return value;
    }

    template <typename T>
    static typename ConfigVar<T>::Ptr Lookup(const std::string& name)
    {
        std::shared_lock lock(GetMutex());
        auto             it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void               LoadFromYaml(const YAML::Node& root);
    static void               LoadFromConfigDirectory(const std::string& path, bool force = false);
    static ConfigVarBase::Ptr LookupBase(const std::string& name);
    static void               Visit(std::function<void(ConfigVarBase::Ptr)> cb);

private:
    static ConfigVarMap& GetDatas()
    {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static std::shared_mutex& GetMutex()
    {
        static std::shared_mutex s_rwmutex;
        return s_rwmutex;
    }
};

} // namespace zed

#endif // ZED_COMM_CONFIG_H_