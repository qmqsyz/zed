#ifndef ZED_LEXICALCAST_H_
#define ZED_LEXICALCAST_H_

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

namespace zed {

template <class S, class D>
class LexicalCast {
public:
    D operator()(const S &s) { return boost::lexical_cast<D>(s); }
};

// for vector

template <class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::vector<T> vec;
        std::stringstream ss;
        for (size_t i{0}; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template <class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T> &vec) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : vec) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// for list

template <class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::list<T> lst;
        std::stringstream ss;
        for (size_t i{0}; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            lst.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return lst;
    }
};

template <class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T> &lst) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : lst) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// for set

template <class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::set<T> st;
        std::stringstream ss;
        for (size_t i{0}; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            st.emplace(LexicalCast<std::string, T>()(ss.str()));
        }
        return st;
    }
};

template <class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T> &st) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : st) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// for unordered_set

template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::unordered_set<T> st;
        std::stringstream ss;
        for (size_t i{0}; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            st.emplace(LexicalCast<std::string, T>()(ss.str()));
        }
        return st;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T> &st) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto &i : st) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// for map

template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::map<std::string, T> mp;
        std::stringstream ss;
        for (auto &[k, v] : mp) {
            ss.str("");
            ss << v;
            mp.emplace(k, LexicalCast<std::string, T>()(ss.str()));
        }
        return mp;
    }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T> &mp) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto &[k, v] : mp) {
            node[k] = YAML::Load(LexicalCast<T, std::string>()(v));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// for unoredered_map

template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string &str) {
        YAML::Node node = YAML::Load(str);
        std::unordered_map<std::string, T> mp;
        std::stringstream ss;
        for (auto &[k, v] : mp) {
            ss.str("");
            ss << v;
            mp.emplace(k, LexicalCast<std::string, T>()(ss.str()));
        }
        return mp;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T> &mp) {
        YAML::Node node(YAML::NodeType::Map);
        for (auto &[k, v] : mp) {
            node[k] = YAML::Load(LexicalCast<T, std::string>()(v));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

} // namespace zed

#endif // ZED_LEXICALCAST_H_