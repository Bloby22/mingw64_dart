#include "Utils/files.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Utils {
    namespace {
        // SHA-256 state used to validate downloaded SDK archives
        struct Sha256Ctx {
            std::uint32_t h[8];
            std::uint64_t len = 0;
            std::array<std::uint8_t, 64> buf{};
            std::size_t bufLen = 0;
        };

        constexpr std::uint32_t K[64] = {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
        };

        std::uint32_t Rotr(std::uint32_t x, int n) {
            return (x >> n) | (x << (32 - n));
        }

        void ShaCompress(Sha256Ctx& ctx, const std::uint8_t block[64]) {
            std::uint32_t w[64];
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
                       (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
                       (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
                       (static_cast<std::uint32_t>(block[i * 4 + 3]));
            }
            for (int i = 16; i < 64; ++i) {
                std::uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                std::uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            std::uint32_t a = ctx.h[0], b = ctx.h[1], c = ctx.h[2], d = ctx.h[3];
            std::uint32_t e = ctx.h[4], f = ctx.h[5], g = ctx.h[6], hh = ctx.h[7];

            for (int i = 0; i < 64; ++i) {
                std::uint32_t S1 = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
                std::uint32_t ch = (e & f) ^ (~e & g);
                std::uint32_t t1 = hh + S1 + ch + K[i] + w[i];
                std::uint32_t S0 = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
                std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                std::uint32_t t2 = S0 + maj;
                hh = g; g = f; f = e; e = d + t1;
                d = c; c = b; b = a; a = t1 + t2;
            }

            ctx.h[0] += a; ctx.h[1] += b; ctx.h[2] += c; ctx.h[3] += d;
            ctx.h[4] += e; ctx.h[5] += f; ctx.h[6] += g; ctx.h[7] += hh;
        }

        void ShaInit(Sha256Ctx& ctx) {
            ctx.h[0] = 0x6a09e667u; ctx.h[1] = 0xbb67ae85u;
            ctx.h[2] = 0x3c6ef372u; ctx.h[3] = 0xa54ff53au;
            ctx.h[4] = 0x510e527fu; ctx.h[5] = 0x9b05688cu;
            ctx.h[6] = 0x1f83d9abu; ctx.h[7] = 0x5be0cd19u;
            ctx.len = 0;
            ctx.bufLen = 0;
        }

        void ShaUpdate(Sha256Ctx& ctx, const void* data, std::size_t n) {
            const auto* p = static_cast<const std::uint8_t*>(data);
            ctx.len += n;
            while (n > 0) {
                std::size_t take = 64 - ctx.bufLen;
                if (take > n) take = n;
                for (std::size_t i = 0; i < take; ++i) ctx.buf[ctx.bufLen + i] = p[i];
                ctx.bufLen += take;
                p += take;
                n -= take;
                if (ctx.bufLen == 64) {
                    ShaCompress(ctx, ctx.buf.data());
                    ctx.bufLen = 0;
                }
            }
        }

        std::string ShaFinish(Sha256Ctx& ctx) {
            std::uint64_t bits = ctx.len * 8;
            std::uint8_t pad = 0x80;
            ShaUpdate(ctx, &pad, 1);
            std::uint8_t zero = 0;
            while (ctx.bufLen != 56) ShaUpdate(ctx, &zero, 1);
            std::uint8_t lenBuf[8];
            for (int i = 0; i < 8; ++i) lenBuf[i] = static_cast<std::uint8_t>(bits >> (56 - i * 8));
            ctx.len = 0;
            ShaUpdate(ctx, lenBuf, 8);

            std::ostringstream os;
            os << std::hex << std::setfill('0');
            for (int i = 0; i < 8; ++i) os << std::setw(8) << ctx.h[i];
            return os.str();
        }

        std::string HexDigestOfFile(const std::filesystem::path& p) {
            std::ifstream in(p, std::ios::binary);
            if (!in) throw std::runtime_error("Nelze otevrit pro cteni: " + p.string());

            Sha256Ctx ctx;
            ShaInit(ctx);

            std::array<char, 64 * 1024> buf{};
            while (in.read(buf.data(), static_cast<std::streamsize>(buf.size())) || in.gcount() > 0) {
                ShaUpdate(ctx, buf.data(), static_cast<std::size_t>(in.gcount()));
            }
            return ShaFinish(ctx);
        }
    }

    std::uintmax_t FileSizeOf(const std::filesystem::path& p) {
        std::error_code ec;
        auto sz = std::filesystem::file_size(p, ec);
        return ec ? 0 : sz;
    }

    bool FileExists(const std::filesystem::path& p) {
        std::error_code ec;
        return std::filesystem::exists(p, ec);
    }

    std::string HumanSize(std::uintmax_t bytes) {
        constexpr std::uintmax_t KiB = 1024;
        constexpr std::uintmax_t MiB = 1024 * KiB;
        constexpr std::uintmax_t GiB = 1024 * MiB;

        std::ostringstream os;
        os << std::fixed << std::setprecision(1);
        if (bytes >= GiB)      os << static_cast<double>(bytes) / GiB << " GB";
        else if (bytes >= MiB) os << static_cast<double>(bytes) / MiB << " MB";
        else if (bytes >= KiB) os << static_cast<double>(bytes) / KiB << " KB";
        else                   os << bytes << " B";
        return os.str();
    }

    std::string Sha256File(const std::filesystem::path& p) {
        return HexDigestOfFile(p);
    }

    std::string Sha256String(const std::string& data) {
        Sha256Ctx ctx;
        ShaInit(ctx);
        ShaUpdate(ctx, data.data(), data.size());
        return ShaFinish(ctx);
    }
}
