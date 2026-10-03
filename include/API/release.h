#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

namespace API {
    struct DartSdkVersion {
        std::string version;
        std::string date;
    };

    // Read channels/<channel>/release/latest/VERSION from dart-archive
    DartSdkVersion FetchDartLatestVersion(const std::string& channel = "stable");

    // Download the SDK archive and verify its SHA-256 when a checksum file exists
    void DownloadDartSdk(const std::string& version,
                         const std::filesystem::path& destDir,
                         const std::string& channel = "stable",
                         const std::string& os = "windows",
                         const std::string& arch = "x64",
                         const std::function<void(std::uint64_t, std::uint64_t)>& progress = nullptr);
}
