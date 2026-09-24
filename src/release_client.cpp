#include "API/google_storage.h"
#include "API/release.h"
#include "Utils/files.h"
#include "Utils/strings.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {
    const char* AppVersion() {
#ifdef APP_VERSION
        return APP_VERSION;
#else
        return "development";
#endif
    }

    void PrintUsage() {
        std::printf(
            "flutter - downloader for Dart SDK archives and Flutter release info\n"
            "Version: %s\n"
            "\n"
            "Usage:\n"
            "  flutter releases [--platform <platform>] [--channel <channel>] [--limit <n>]\n"
            "      List releases from releases_<platform>.json (default: windows, stable, 10).\n"
            "  flutter dart-latest [--channel <channel>]\n"
            "      Print the newest Dart SDK version from dart-archive (default: stable).\n"
            "  flutter dartsdk [--dest <dir>] [--channel <channel>]\n"
            "      Download and extract the latest Dart SDK for Windows x64.\n"
            "  flutter sdk <version> [--dest <dir>] [--channel <channel>]\n"
            "                  [--os <os>] [--arch <arch>]\n"
            "      Download dartsdk-<os>-<arch>-release.zip and verify SHA-256.\n"
            "      Use --extract to extract into <dir>/dart-sdk, or --no-extract.\n"
            "  flutter sha256 <file>\n"
            "      Compute the SHA-256 of a file.\n", AppVersion());
    }

    std::string ArgValue(const std::vector<std::string>& args,
                         std::size_t& i, const std::string& flag) {
        if (i + 1 >= args.size()) throw API::Error("missing value after " + flag);
        return args[++i];
    }

    std::string PowerShellQuote(const std::filesystem::path& path) {
        std::string value = path.string();
        if (value.size() >= 3 && value[0] == '/' &&
            value[1] >= 'a' && value[1] <= 'z' && value[2] == '/') {
            value = std::string(1, static_cast<char>(value[1] - ('a' - 'A'))) + ":" + value.substr(2);
        }
        std::string escaped;
        for (char c : value) escaped += c == '\'' ? "''" : std::string(1, c);
        return "'" + escaped + "'";
    }

    std::string MsysPath(const std::filesystem::path& path) {
        std::string value = path.string();
        if (value.size() >= 2 && value[1] == ':') {
            value = "/" + Utils::ToLower(value.substr(0, 1)) + value.substr(2);
        }
        for (char& c : value) if (c == '\\') c = '/';
        return value;
    }

    void ExtractSdk(const std::filesystem::path& zipPath,
                    const std::filesystem::path& dest) {
        std::string command = "powershell.exe -NoProfile -NonInteractive -Command "
            "\"Expand-Archive -LiteralPath " + PowerShellQuote(zipPath) +
            " -DestinationPath " + PowerShellQuote(dest) + " -Force\"";
        if (std::system(command.c_str()) != 0) {
            throw API::Error("failed to extract the Dart SDK archive");
        }
    }

    void ProgressBar(std::uint64_t done, std::uint64_t total) {
        if (total == 0) {
            std::printf("\rDownloaded %s   ", Utils::HumanSize(done).c_str());
            return;
        }
        double ratio = static_cast<double>(done) / static_cast<double>(total);
        int filled = static_cast<int>(ratio * 30.0);
        std::printf("\r[");
        for (int i = 0; i < 30; ++i) std::printf(i < filled ? "#" : "-");
        std::printf("] %s / %s   ",
                    Utils::HumanSize(done).c_str(),
                    Utils::HumanSize(total).c_str());
        if (done == total) std::printf("\n");
        std::fflush(stdout);
    }

    int CmdFlutter(const std::vector<std::string>& args) {
        std::string platform = "windows";
        std::string channel = "stable";
        int limit = 10;

        for (std::size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--platform") platform = ArgValue(args, i, args[i]);
            else if (args[i] == "--channel") channel = ArgValue(args, i, args[i]);
            else if (args[i] == "--limit") limit = std::stoi(ArgValue(args, i, args[i]));
            else throw API::Error("unknown option: " + args[i]);
        }

        auto all = API::FetchFlutterReleases(platform);
        std::printf("base_url: %s\n", all.base_url.c_str());
        std::printf("%-12s %-10s %-16s %-34s %s\n",
                    "channel", "version", "dart", "hash", "date");
        int shown = 0;
        for (const auto& r : all.releases) {
            if (!channel.empty() && Utils::ToLower(r.channel) != Utils::ToLower(channel)) continue;
            if (shown++ >= limit) break;
            std::printf("%-12s %-10s %-16s %-34s %lld\n",
                        r.channel.c_str(), r.version.c_str(), r.dart_sdk_version.c_str(),
                        Utils::Trim(r.hash).substr(0, 32).c_str(),
                        static_cast<long long>(r.release_date_ms));
        }
        if (shown == 0) {
            std::printf("No releases for channel \"%s\".\n", channel.c_str());
            return 1;
        }
        return 0;
    }

    int CmdDartLatest(const std::vector<std::string>& args) {
        std::string channel = "stable";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--channel") channel = ArgValue(args, i, args[i]);
            else throw API::Error("unknown option: " + args[i]);
        }
        auto v = API::FetchDartLatestVersion(channel);
        std::printf("%s (%s)\n", v.version.c_str(), v.date.c_str());
        return 0;
    }

    int CmdSdk(const std::vector<std::string>& args, bool extract = false) {
        if (args.empty()) throw API::Error("missing SDK version (or \"latest\")");

        std::string version = args[0];
        std::string channel = "stable";
        std::string os = "windows";
        std::string arch = "x64";
        std::filesystem::path dest = std::filesystem::current_path();

        for (std::size_t i = 1; i < args.size(); ++i) {
            if (args[i] == "--dest") dest = ArgValue(args, i, args[i]);
            else if (args[i] == "--channel") channel = ArgValue(args, i, args[i]);
            else if (args[i] == "--os") os = ArgValue(args, i, args[i]);
            else if (args[i] == "--arch") arch = ArgValue(args, i, args[i]);
            else if (args[i] == "--extract") extract = true;
            else if (args[i] == "--no-extract") extract = false;
            else throw API::Error("unknown option: " + args[i]);
        }

        if (Utils::ToLower(version) == "latest") {
            auto v = API::FetchDartLatestVersion(channel);
            version = v.version;
            std::printf("Newest version for channel %s: %s\n", channel.c_str(), version.c_str());
        }

        API::DownloadDartSdk(version, dest, channel, os, arch, ProgressBar);
        if (extract) {
            std::string zipName = "dartsdk-" + Utils::ToLower(os) + "-" +
                                  Utils::ToLower(arch) + "-release.zip";
            std::filesystem::path zipPath = dest / zipName;
            std::filesystem::path sdkBin = dest / "dart-sdk" / "bin";
            std::printf("Extracting %s\n", zipPath.string().c_str());
            ExtractSdk(zipPath, dest);
            std::printf("Dart SDK extracted to: %s\n", (dest / "dart-sdk").string().c_str());
            std::printf("Add to MSYS2 PATH: export PATH=\"%s:$PATH\"\n",
                        MsysPath(sdkBin).c_str());
        }
        return 0;
    }

    int CmdSha256(const std::vector<std::string>& args) {
        if (args.empty()) throw API::Error("missing file path");
        std::printf("%s  %s\n",
                    Utils::Sha256File(args[0]).c_str(), args[0].c_str());
        return 0;
    }
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        PrintUsage();
        return 2;
    }

    std::string cmd = args[0];
    std::vector<std::string> rest(args.begin() + 1, args.end());

    try {
        if (cmd == "--version" || cmd == "-V" || cmd == "version") {
            std::printf("flutter %s\n", AppVersion());
            return 0;
        }
        if (cmd == "releases" || cmd == "flutter") return CmdFlutter(rest);
        if (cmd == "dart-latest")  return CmdDartLatest(rest);
        if (cmd == "sdk" || cmd == "dartsdk") {
            bool extract = cmd == "dartsdk";
            if (extract && (rest.empty() || rest.front().rfind("--", 0) == 0)) {
                rest.insert(rest.begin(), "latest");
            }
            return CmdSdk(rest, extract);
        }
        if (cmd == "sha256")       return CmdSha256(rest);
        if (cmd == "help" || cmd == "--help" || cmd == "-h") {
            PrintUsage();
            return 0;
        }
        std::fprintf(stderr, "Unknown command: %s\n\n", cmd.c_str());
        PrintUsage();
        return 2;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
}
