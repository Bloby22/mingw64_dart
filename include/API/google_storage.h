#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>

namespace API {
    inline constexpr const char* kHost = "https://storage.googleapis.com";
    inline constexpr const char* kFlutterBucket = "flutter_infra_release";
    inline constexpr const char* kDartHost = "https://storage.googleapis.com/dart-archive";

    class Error : public std::runtime_error {
        public:
        using std::runtime_error::runtime_error;
    };

    // Build object URL
    std::string ObjectUrl(const std::string& bucket, const std::string& object);

    // Fetch text from URL
    std::string GetText(const std::string& url);

    using Progress = std::function<void(std::uint64_t done, std::uint64_t total)>;

    // Download file to destination
    void Download(const std::string& url, const std::filesystem::path& dest,
                Progress progress = nullptr);
}
