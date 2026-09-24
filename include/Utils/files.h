#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

namespace Utils {
    // Return the file size in bytes, or 0 if the file does not exist.
    std::uintmax_t FileSizeOf(const std::filesystem::path& p);

    bool FileExists(const std::filesystem::path& p);

    // 1234567 -> "1.2 MB"
    std::string HumanSize(std::uintmax_t bytes);

    // Calculate the file content SHA-256 as a 64-character hexadecimal string.
    std::string Sha256File(const std::filesystem::path& p);

    // Calculate the string SHA-256 as a 64-character hexadecimal string.
    std::string Sha256String(const std::string& data);
}
