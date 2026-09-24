#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace API {
    // One published Flutter SDK artifact
    struct FlutterRelease {
        std::string hash;
        std::string channel;
        std::string version;
        std::string dart_sdk_version;
        std::string release_note;
        std::string archive_path;
        std::int64_t release_date_ms = 0;
    };

    // Release list plus the currently selected artifact
    struct FlutterReleases {
        std::string base_url;
        std::vector<FlutterRelease> releases;
        std::vector<FlutterRelease> current_release;
    };

    // Download and parse releases_<platform>.json from Flutter storage
    FlutterReleases FetchFlutterReleases(const std::string& platform = "windows");

    // Empty version means newest release for the given channel
    const FlutterRelease* FindRelease(const FlutterReleases& all,
                                      const std::string& channel,
                                      const std::string& version = "");

    // Build archive URL from base_url and archive_path
    std::string ReleaseArchiveUrl(const FlutterReleases& all, const FlutterRelease& r);

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
