#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace Utils {
    std::string Trim(const std::string& s);
    std::string TrimLeft(const std::string& s);
    std::string TrimRight(const std::string& s);

    std::vector<std::string> Split(const std::string& s, char sep);
    std::vector<std::string> SplitAny(const std::string& s, const std::string& seps);
    std::string Join(const std::vector<std::string>& parts, const std::string& sep);

    bool StartsWith(const std::string& s, const std::string& prefix);
    bool EndsWith(const std::string& s, const std::string& suffix);

    std::string ReplaceAll(std::string s, const std::string& from, const std::string& to);
    std::string ToLower(std::string s);
    std::string ToUpper(std::string s);
}
