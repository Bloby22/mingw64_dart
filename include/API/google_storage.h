#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>

namespace API {
    inline constexpr const char* kHost = "https://storage.googleapis.com";
    inline constexpr const char* kFlutterBucket = "flutter_infra_release";

    class Error : public std::runtime_error {
        public:
        using std::runtime_error::runtime_error;
    };

    // Build 
    std::string ObjectUrl(const std::string& bucket, const std::string& object);

    // Download new object (e.g. JSON) to memory
    std::string GetText(const std::string& url);

    using Progress = std::function<void(std::uint64_t done, std::uint64_t total)>;

    // Download to dest (for dest + ".part" for done renaming)
    void Download(const std::string& url, const std::filesystem::path& dest,
                Progress progress = nullptr);
}
