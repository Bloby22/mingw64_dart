#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Utils {
    class Json;
    using JsonPtr = std::unique_ptr<Json>;

    class Json {
        public:
        enum class Type { Null, Bool, Number, String, Array, Object };

        Type type = Type::Null;
        bool boolean = false;
        double number = 0.0;
        std::string text;
        std::vector<JsonPtr> array;
        std::vector<std::pair<std::string, JsonPtr>> object;

        bool IsNull() const { return type == Type::Null; }
        bool IsBool() const { return type == Type::Bool; }
        bool IsNumber() const { return type == Type::Number; }
        bool IsString() const { return type == Type::String; }
        bool IsArray() const { return type == Type::Array; }
        bool IsObject() const { return type == Type::Object; }

        const Json* Find(const char* key) const {
            if (type != Type::Object) return nullptr;
            for (const auto& kv : object)
                if (kv.first == key) return kv.second.get();
            return nullptr;
        }

        std::string StringOr(const char* key, const std::string& fallback = {}) const {
            const Json* v = Find(key);
            return (v && v->IsString()) ? v->text : fallback;
        }

        std::string NumberAsStringOr(const char* key, const std::string& fallback = {}) const {
            const Json* v = Find(key);
            if (!v) return fallback;
            if (v->IsString()) return v->text;
            if (v->IsNumber()) return std::to_string(v->number);
            return fallback;
        }

        std::string Dump(std::size_t indent = 0) const;
    };

    // Parse the text; on failure, throw std::runtime_error with the position.
    JsonPtr JsonParse(const std::string& text);
}
