#include "API/google_storage.h"
#include "API/release.h"
#include "Utils/files.h"
#include "Utils/strings.h"
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {
    void PrintUsage() {
        std::printf(
            "mingw64-dart - nastroj pro stahovani Dart SDK a info o Flutter vydani\n"
            "\n"
            "Pouziti:\n"
            "  release_client flutter [--platform <platform>] [--channel <kanal>] [--limit <n>]\n"
            "      Vypise vydani z releases_<platform>.json (vychozi: windows, stable, 10).\n"
            "  release_client dart-latest [--channel <kanal>]\n"
            "      Vypise nejnovejsi verzi Dart SDK z dart-archive (vychozi: stable).\n"
            "  release_client sdk <verze> [--dest <adresar>] [--channel <kanal>]\n"
            "                  [--os <os>] [--arch <arch>]\n"
            "      Stahne dartsdk-<os>-<arch>-release.zip a overi SHA-256.\n"
            "      Verze muze byt \"latest\" (vychozi) nebo konkretni, napr. 3.9.4.\n"
            "  release_client sha256 <soubor>\n"
            "      Spocte SHA-256 souboru.\n");
    }

    std::string ArgValue(const std::vector<std::string>& args,
                         std::size_t& i, const std::string& flag) {
        if (i + 1 >= args.size()) throw API::Error("chybi hodnota za " + flag);
        return args[++i];
    }

    void ProgressBar(std::uint64_t done, std::uint64_t total) {
        if (total == 0) {
            std::printf("\rStazeno %s   ", Utils::HumanSize(done).c_str());
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
            else throw API::Error("neznamy prepinac: " + args[i]);
        }

        auto all = API::FetchFlutterReleases(platform);
        std::printf("base_url: %s\n", all.base_url.c_str());
        std::printf("%-12s %-10s %-16s %-34s %s\n",
                    "kanal", "verze", "dart", "hash", "datum");
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
            std::printf("Zadna vydani pro kanal \"%s\".\n", channel.c_str());
            return 1;
        }
        return 0;
    }

    int CmdDartLatest(const std::vector<std::string>& args) {
        std::string channel = "stable";
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (args[i] == "--channel") channel = ArgValue(args, i, args[i]);
            else throw API::Error("neznamy prepinac: " + args[i]);
        }
        auto v = API::FetchDartLatestVersion(channel);
        std::printf("%s (%s)\n", v.version.c_str(), v.date.c_str());
        return 0;
    }

    int CmdSdk(const std::vector<std::string>& args) {
        if (args.empty()) throw API::Error("chybi verze SDK (nebo \"latest\")");

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
            else throw API::Error("neznamy prepinac: " + args[i]);
        }

        if (Utils::ToLower(version) == "latest") {
            auto v = API::FetchDartLatestVersion(channel);
            version = v.version;
            std::printf("Nejnovejsi verze kanalu %s: %s\n", channel.c_str(), version.c_str());
        }

        API::DownloadDartSdk(version, dest, channel, os, arch, ProgressBar);
        return 0;
    }

    int CmdSha256(const std::vector<std::string>& args) {
        if (args.empty()) throw API::Error("chybi cesta k souboru");
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
        if (cmd == "flutter")      return CmdFlutter(rest);
        if (cmd == "dart-latest")  return CmdDartLatest(rest);
        if (cmd == "sdk")          return CmdSdk(rest);
        if (cmd == "sha256")       return CmdSha256(rest);
        if (cmd == "help" || cmd == "--help" || cmd == "-h") {
            PrintUsage();
            return 0;
        }
        std::fprintf(stderr, "Neznamy prikaz: %s\n\n", cmd.c_str());
        PrintUsage();
        return 2;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Chyba: %s\n", e.what());
        return 1;
    }
}
