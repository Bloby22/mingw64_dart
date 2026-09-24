#include "Utils/json.h"
#include "Utils/strings.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace Utils {
    namespace {
        // Minimal JSON parser for the release metadata used by this project
        class Parser {
            public:
            explicit Parser(const std::string& text) : s_(text) {}

            JsonPtr Run() {
                SkipWs();
                auto v = Value(0);
                SkipWs();
                if (pos_ != s_.size()) Fail("necekana data za hodnotou");
                return v;
            }

            private:
            const std::string& s_;
            std::size_t pos_ = 0;

            [[noreturn]] void Fail(const char* what) const {
                throw std::runtime_error(std::string(what) + " (pozice " + std::to_string(pos_) + ")");
            }

            void SkipWs() {
                while (pos_ < s_.size()) {
                    char c = s_[pos_];
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++pos_;
                    else break;
                }
            }

            char Peek() const {
                if (pos_ >= s_.size()) Fail("neocekavany konec vstupu");
                return s_[pos_];
            }

            void Expect(char c) {
                if (pos_ >= s_.size() || s_[pos_] != c)
                    Fail(std::string("cekam '").append(1, c).append("'").c_str());
                ++pos_;
            }

            bool ConsumeIf(char c) {
                if (pos_ < s_.size() && s_[pos_] == c) {
                    ++pos_;
                    return true;
                }
                return false;
            }

            JsonPtr Value(int depth) {
                if (depth > 64) Fail("prilis hluboke zanoreni");
                SkipWs();
                char c = Peek();
                if (c == '{') return Object(depth);
                if (c == '[') return Array(depth);
                if (c == '"') {
                    auto v = std::make_unique<Json>();
                    v->type = Json::Type::String;
                    v->text = String();
                    return v;
                }
                if (c == 't') { Literal("true");  return BoolV(true); }
                if (c == 'f') { Literal("false"); return BoolV(false); }
                if (c == 'n') { Literal("null");  return std::make_unique<Json>(); }
                return NumberV();
            }

            JsonPtr BoolV(bool b) {
                auto v = std::make_unique<Json>();
                v->type = Json::Type::Bool;
                v->boolean = b;
                return v;
            }

            void Literal(const char* lit) {
                for (const char* p = lit; *p; ++p) {
                    if (pos_ >= s_.size() || s_[pos_] != *p) Fail("neplatny literal");
                    ++pos_;
                }
            }

            JsonPtr NumberV() {
                std::size_t start = pos_;
                if (ConsumeIf('-')) {}
                while (pos_ < s_.size() && std::isdigit(static_cast<unsigned char>(s_[pos_]))) ++pos_;
                if (ConsumeIf('.')) {
                    while (pos_ < s_.size() && std::isdigit(static_cast<unsigned char>(s_[pos_]))) ++pos_;
                }
                if (pos_ < s_.size() && (s_[pos_] == 'e' || s_[pos_] == 'E')) {
                    ++pos_;
                    ConsumeIf('+') || ConsumeIf('-');
                    while (pos_ < s_.size() && std::isdigit(static_cast<unsigned char>(s_[pos_]))) ++pos_;
                }
                if (pos_ == start) Fail("neplatne cislo");
                auto v = std::make_unique<Json>();
                v->type = Json::Type::Number;
                try {
                    v->number = std::stod(s_.substr(start, pos_ - start));
                } catch (...) {
                    Fail("neplatne cislo");
                }
                return v;
            }

            std::string String() {
                Expect('"');
                std::string out;
                while (true) {
                    if (pos_ >= s_.size()) Fail("neocekavany konec vstupu");
                    char c = s_[pos_++];
                    if (c == '"') break;
                    if (c == '\\') {
                        if (pos_ >= s_.size()) Fail("neocekavany konec vstupu");
                        char e = s_[pos_++];
                        switch (e) {
                            case '"': out.push_back('"'); break;
                            case '\\': out.push_back('\\'); break;
                            case '/': out.push_back('/'); break;
                            case 'b': out.push_back('\b'); break;
                            case 'f': out.push_back('\f'); break;
                            case 'n': out.push_back('\n'); break;
                            case 'r': out.push_back('\r'); break;
                            case 't': out.push_back('\t'); break;
                            case 'u': out.push_back(UnicodeEscape()); break;
                            default: Fail("neplatny escape");
                        }
                    } else {
                        out.push_back(c);
                    }
                }
                return out;
            }

            char UnicodeEscape() {
                unsigned code = Hex4();
                if (code < 0x80) return static_cast<char>(code);
                Fail("escape > 0x7F neni podporovan");
            }

            unsigned Hex4() {
                if (pos_ + 4 > s_.size()) Fail("neplatny \\u escape");
                unsigned v = 0;
                for (int i = 0; i < 4; ++i) {
                    char c = s_[pos_++];
                    v <<= 4;
                    if (c >= '0' && c <= '9') v |= static_cast<unsigned>(c - '0');
                    else if (c >= 'a' && c <= 'f') v |= static_cast<unsigned>(c - 'a' + 10);
                    else if (c >= 'A' && c <= 'F') v |= static_cast<unsigned>(c - 'A' + 10);
                    else Fail("neplatny \\u escape");
                }
                return v;
            }

            JsonPtr Array(int depth) {
                Expect('[');
                auto v = std::make_unique<Json>();
                v->type = Json::Type::Array;
                SkipWs();
                if (ConsumeIf(']')) return v;
                while (true) {
                    v->array.push_back(Value(depth + 1));
                    SkipWs();
                    if (ConsumeIf(']')) break;
                    Expect(',');
                }
                return v;
            }

            JsonPtr Object(int depth) {
                Expect('{');
                auto v = std::make_unique<Json>();
                v->type = Json::Type::Object;
                SkipWs();
                if (ConsumeIf('}')) return v;
                while (true) {
                    SkipWs();
                    std::string key = String();
                    SkipWs();
                    Expect(':');
                    v->object.emplace_back(std::move(key), Value(depth + 1));
                    SkipWs();
                    if (ConsumeIf('}')) break;
                    Expect(',');
                }
                return v;
            }
        };
    }

    std::string Json::Dump(std::size_t indent) const {
        auto pad = [&](std::size_t n) { return std::string(n * 2, ' '); };
        std::string out;
        switch (type) {
            case Type::Null: out = "null"; break;
            case Type::Bool: out = boolean ? "true" : "false"; break;
            case Type::Number: {
                if (std::floor(number) == number &&
                    std::abs(number) < 1e15) {
                    out = std::to_string(static_cast<long long>(number));
                } else {
                    out = std::to_string(number);
                }
                break;
            }
            case Type::String: {
                out = "\"";
                for (char c : text) {
                    switch (c) {
                        case '"': out += "\\\""; break;
                        case '\\': out += "\\\\"; break;
                        case '\n': out += "\\n"; break;
                        case '\r': out += "\\r"; break;
                        case '\t': out += "\\t"; break;
                        default: out.push_back(c); break;
                    }
                }
                out += "\"";
                break;
            }
            case Type::Array: {
                out = "[\n";
                for (std::size_t i = 0; i < array.size(); ++i) {
                    out += pad(indent + 1);
                    out += array[i]->Dump(indent + 1);
                    if (i + 1 != array.size()) out += ",";
                    out += "\n";
                }
                out += pad(indent);
                out += "]";
                break;
            }
            case Type::Object: {
                out = "{\n";
                for (std::size_t i = 0; i < object.size(); ++i) {
                    out += pad(indent + 1);
                    Json k;
                    k.type = Type::String;
                    k.text = object[i].first;
                    out += k.Dump(indent + 1);
                    out += ": ";
                    out += object[i].second->Dump(indent + 1);
                    if (i + 1 != object.size()) out += ",";
                    out += "\n";
                }
                out += pad(indent);
                out += "}";
                break;
            }
        }
        return out;
    }

    JsonPtr JsonParse(const std::string& text) {
        Parser p(text);
        return p.Run();
    }
}
