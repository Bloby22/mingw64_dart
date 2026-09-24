#include "Utils/strings.h"
#include <algorithm>
#include <cctype>

namespace Utils {
    namespace {
        int Space(int c) {
            return std::isspace(static_cast<unsigned char>(c)) != 0;
        }
    }

    std::string TrimLeft(const std::string& s) {
        auto b = s.begin();
        while (b != s.end() && Space(*b)) ++b;
        return std::string(b, s.end());
    }

    std::string TrimRight(const std::string& s) {
        auto e = s.end();
        while (e != s.begin() && Space(*(e - 1))) --e;
        return std::string(s.begin(), e);
    }

    std::string Trim(const std::string& s) {
        return TrimRight(TrimLeft(s));
    }

    std::vector<std::string> Split(const std::string& s, char sep) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == sep) {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        out.push_back(cur);
        return out;
    }

    std::vector<std::string> SplitAny(const std::string& s, const std::string& seps) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (seps.find(c) != std::string::npos) {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        out.push_back(cur);
        return out;
    }

    std::string Join(const std::vector<std::string>& parts, const std::string& sep) {
        std::string out;
        for (std::size_t i = 0; i < parts.size(); ++i) {
            if (i != 0) out += sep;
            out += parts[i];
        }
        return out;
    }

    bool StartsWith(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    bool EndsWith(const std::string& s, const std::string& suffix) {
        return s.size() >= suffix.size() &&
               s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    std::string ReplaceAll(std::string s, const std::string& from, const std::string& to) {
        if (from.empty()) return s;
        std::size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.size(), to);
            pos += to.size();
        }
        return s;
    }

    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    std::string ToUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return s;
    }
}
