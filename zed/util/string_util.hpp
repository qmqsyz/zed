#ifndef ZED_UTIL_STRINGUTIL_HPP_
#define ZED_UTIL_STRINGUTIL_HPP_

#include <map>
#include <string>
#include <vector>

#include <iostream>

namespace zed {

namespace util {

    void SplitStrToVector(const std::string_view&   str,
                          const std::string_view&   split_str,
                          std::vector<std::string>& res)
    {
        if (str.empty() || split_str.empty()) [[unlikely]] {
            return;
        }
        std::string s {str};
        if (str.substr(str.size() - split_str.size(), split_str.size()) != split_str) {
            s.append(split_str);
        }

        std::string_view tmp(s);
        while (true) {
            auto i = tmp.find_first_of(split_str);
            if (i == tmp.npos) {
                return;
            }
            std::string x {tmp.substr(0, i)};
            tmp = tmp.substr(i + split_str.size());
            if (!x.empty()) {
                res.push_back(std::move(x));
            }
        }
    }

    void SplitStrToMap(const std::string_view&             str,
                       const std::string_view&             split_str,
                       const std::string_view&             joiner,
                       std::map<std::string, std::string>& res)
    {
        if (str.empty() || split_str.empty() || joiner.empty()) [[unlikely]] {
            return;
        }
        std::vector<std::string> vec;
        SplitStrToVector(str, split_str, vec);
        for (auto& i : vec) {
            auto j = i.find_first_of(joiner);
            if (j != i.npos && j != 0) {
                std::string key = i.substr(0, j);
                std::string value = i.substr(j + joiner.size());
                res.emplace(std::move(key), std::move(value));
            }
        }
    }

} // namespace util

} // namespace zed

#endif // ZED_UTIL_STRINGUTIL_HPP_
