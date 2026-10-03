#include "API/release.h"
#include "API/google_storage.h"
#include "Utils/files.h"
#include "Utils/json.h"
#include "Utils/strings.h"
#include <cstdio>
#include <system_error>

namespace API {
    namespace {
        std::string TrimSha(const std::string& s) {
            auto parts = Utils::SplitAny(s, " \t\r\n");
            for (const auto& p : parts) {
                if (p.size() == 64) return Utils::ToLower(p);
            }
            return Utils::ToLower(Utils::Trim(s));
        }
    }

    DartSdkVersion FetchDartLatestVersion(const std::string& channel) {
        std::string url = std::string(kDartHost) + "/channels/" + Utils::ToLower(channel) +
                          "/release/latest/VERSION";
        std::string body = GetText(url);

        auto root = Utils::JsonParse(body);
        DartSdkVersion v;
        v.date = root->StringOr("date");
        v.version = root->StringOr("version");
        if (v.version.empty()) throw Error("VERSION does not contain a version");
        return v;
    }

    void DownloadDartSdk(const std::string& version,
                         const std::filesystem::path& destDir,
                         const std::string& channel,
                         const std::string& os,
                         const std::string& arch,
                         const std::function<void(std::uint64_t, std::uint64_t)>& progress) {
        if (version.empty()) throw Error("empty Dart SDK version");
        if (destDir.empty()) throw Error("empty destination directory");

        std::string ch = Utils::ToLower(channel);
        std::string base = std::string(kDartHost) + "/channels/" + ch + "/release/" + version;
        std::string zipName = "dartsdk-" + Utils::ToLower(os) + "-" + Utils::ToLower(arch) +
                              "-release.zip";
        std::string zipUrl = base + "/sdk/" + zipName;

        std::filesystem::create_directories(destDir);
        std::filesystem::path zipPath = destDir / zipName;

        std::printf("Downloading %s\n", zipUrl.c_str());
        Download(zipUrl, zipPath, progress);

        std::string shaUrl = zipUrl + ".sha256sum";
        std::string expected;
        try {
            expected = TrimSha(GetText(shaUrl));
        } catch (const Error&) {
            expected.clear();
        }

        if (!expected.empty()) {
            std::string got = Utils::Sha256File(zipPath);
            if (got != expected) {
                std::error_code ec;
                std::filesystem::remove(zipPath, ec);
                throw Error("SHA-256 mismatch: " + got + " != " + expected);
            }
            std::printf("SHA-256 OK: %s\n", got.c_str());
        } else {
            std::printf("SHA-256 check skipped (%s not found)\n", shaUrl.c_str());
        }

        std::printf("Done: %s\n", zipPath.string().c_str());
    }
}
