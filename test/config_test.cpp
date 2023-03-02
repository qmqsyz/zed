#include <iostream>
#include <yaml-cpp/yaml.h>

#include "zed/comm/config.h"
#include "zed/comm/env.h"
#include "zed/log/log.h"

class Person {
public:
    Person() {};
    std::string m_name;
    int         m_age = 0;
    bool        m_sex = 0;

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[Person name=" << m_name << " age=" << m_age << " sex=" << m_sex << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const
    {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
    }
};

namespace zed {

template <>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v)
    {
        YAML::Node node = YAML::Load(v);
        Person     p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template <>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p)
    {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

} // namespace zed

zed::ConfigVar<Person>::Ptr g_person
    = zed::Config::Lookup("class.person", Person(), "system person");

zed::ConfigVar<std::map<std::string, Person>>::Ptr g_person_map
    = zed::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

zed::ConfigVar<std::map<std::string, std::vector<Person>>>::Ptr g_person_vec_map
    = zed::Config::Lookup(
        "class.vec_map", std::map<std::string, std::vector<Person>>(), "system person");

void test_class()
{
    LOG_INFO << "before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix)                                                       \
    {                                                                              \
        auto m = g_person_map->getValue();                                         \
        for (auto& i : m) {                                                        \
            LOG_INFO << prefix << ": " << i.first << " - " << i.second.toString(); \
        }                                                                          \
        LOG_INFO << prefix << ": size=" << m.size();                               \
    }

    g_person->addListener([](const Person& old_value, const Person& new_value) {
        LOG_INFO << "old_value=" << old_value.toString() << " new_value=" << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");
    LOG_INFO << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("test.yaml");
    zed::Config::LoadFromYaml(root);

    LOG_INFO << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    LOG_INFO << "after: " << g_person_vec_map->toString();
}

void test_loadconf()
{
    zed::Config::LoadFromConfigDirectory("conf");
}

int main(int argc, char** argv)
{
    zed::LoggerManager::GetInstance().addAppender(
        zed::StdoutLogAppender::Ptr(new zed::StdoutLogAppender));
    zed::EnvironmentManager::GetInstance().init(argc, argv);

    // test_yaml();
    // test_config();
    test_class();

    test_loadconf();
    std::cout << " ==== " << std::endl;
    sleep(10);
    test_loadconf();
    return 0;
    // zed::Config::Visit([](zed::ConfigVarBase::Ptr var) {
    //     LOG_INFO << "name=" << var->getName() << " description=" << var->getDescription()
    //              << " typename=" << var->getTypeName() << " value=" << var->toString();
    // });

    return 0;
}
