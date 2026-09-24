#include "API/release.h"
#include "API/google_storage.h"
#include "Utils/files.h"
#include "Utils/json.h"
#include "Utils/strings.h"
#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace API {
    namespace {
        const char* kFlutterReleasesPath = "releases/releases.json";

        std::int64_t DateToMs(const std::string& iso) {
            int y = 0, m = 0, d = 0, hh = 0, mm = 0, ss = 0;
            if (std::sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%dZ",
                            &y, &m, &d, &hh, &mm, &ss) != 6) {
                return 0;
            }
            std::tm t{};
            t.tm_year = y - 1900;
            t.tm_mon = m - 1;
            t.tm_mday = d;
            t.tm_hour = hh;
            t.tm_min = mm;
            t.tm_sec = ss;
            t.tm_isdst = -1;
#ifdef _WIN32
            std::time_t tt = _mkgmtime(&t);
#else
            std::time_t tt = timegm(&t);
#endif
            return tt < 0 ? 0 : static_cast<std::int64_t>(tt) * 1000;
        }

        FlutterRelease ParseRelease(const Utils::Json& j) {
            FlutterRelease r;
            r.hash = j.StringOr("hash");
            r.channel = j.StringOr("channel");
            r.version = j.StringOr("version");
            r.dart_sdk_version = j.StringOr("dart_sdk_version");
            r.release_note = j.StringOr("release_note");
            r.archive_path = j.StringOr("archive");
            r.release_date_ms = DateToMs(j.StringOr("release_date"));
            return r;
        }

        std::string TrimSha(const std::string& s) {
            auto parts = Utils::SplitAny(s, " \t\r\n");
            for (const auto& p : parts) {
                if (p.size() == 64) return Utils::ToLower(p);
            }
            return Utils::ToLower(Utils::Trim(s));
        }
    }

    FlutterReleases FetchFlutterReleases(const std::string& platform) {
        std::string url = std::string(kHost) + "/" + kFlutterBucket + "/releases/releases_" +
                          Utils::ToLower(platform) + ".json";
        std::string body = GetText(url);

        auto root = Utils::JsonParse(body);
        FlutterReleases out;

        const Utils::Json* base = root->Find("base_url");
        out.base_url = (base && base->IsString()) ? base->text : std::string(kHost) + "/" + kFlutterBucket;

        const Utils::Json* cur = root->Find("current_release");
        if (cur && cur->IsObject()) {
            for (const auto& kv : cur->object) {
                FlutterRelease r;
                r.hash = kv.second->IsString() ? kv.second->text : "";
                r.channel = kv.first;
                out.current_release.push_back(std::move(r));
            }
        }

        const Utils::Json* arr = root->Find("releases");
        if (arr && arr->IsArray()) {
            for (const auto& item : arr->array) {
                if (item && item->IsObject()) out.releases.push_back(ParseRelease(*item));
            }
        }

        if (out.releases.empty()) throw Error("releases.json does not contain any releases");
        return out;
    }

    const FlutterRelease* FindRelease(const FlutterReleases& all,
                                      const std::string& channel,
                                      const std::string& version) {
        const FlutterRelease* best = nullptr;
        for (const auto& r : all.releases) {
            if (!Utils::ToLower(r.channel).empty() &&
                Utils::ToLower(r.channel) != Utils::ToLower(channel)) continue;
            if (!version.empty() && r.version != version && r.hash != version) continue;

            if (best == nullptr) {
                best = &r;
                continue;
            }
            if (r.release_date_ms > best->release_date_ms) best = &r;
        }
        return best;
    }

    std::string ReleaseArchiveUrl(const FlutterReleases& all, const FlutterRelease& r) {
        if (r.archive_path.empty()) throw Error("release has no archive path");
        std::string base = all.base_url.empty()
            ? std::string(kHost) + "/" + kFlutterBucket
            : all.base_url;
        if (!base.empty() && base.back() == '/') base.pop_back();
        std::string path = r.archive_path;
        while (!path.empty() && path.front() == '/') path.erase(0, 1);
        return base + "/" + path;
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
