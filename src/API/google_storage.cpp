#include "API/google_storage.h"
#include "curl/curl.h"
#include <fstream>

namespace API {
    namespace {
        struct CurlGlobal {
            
            CurlGlobal() { 
                curl_global_init(CURL_GLOBAL_DEFAULT); 
            }

            ~CurlGlobal() {
                curl_global_cleanup();
            }
        };

        void EnsureCurl() {
            static CurlGlobal g;
        }

        struct Easy {
            CURL *h;
            Easy() : h(curl_easy_init()) {
                if (!h) throw Error("curl_easy_init crashed");
                curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(h, CURLOPT_FAILONERROR, 1L);
                curl_easy_setopt(h, CURLOPT_USERAGENT, "mingw64-dart/0.1");
            }

            ~Easy() {
                curl_easy_cleanup(h);
            }

            Easy(const Easy&) = delete;
            Easy& operator=(const Easy&) = delete;

            void Perform(const char* what) {
                CURLcode rc = curl_easy_perform(h);
                if (rc != CURLE_OK) throw Error(std::string(what) + ": " + curl_easy_strerror(rc));
            }
        };

        std::size_t WriteToString(char* p, std::size_t sz, std::size_t n, void* ud) {
            static_cast<std::string*>(ud)->append(p, sz * n);
            return sz * n;
        }

        std::size_t WriteToFile(char* p, std::size_t sz, std::size_t n, void* ud) {
            auto* out = static_cast<std::ofstream*>(ud);
            out->write(p, static_cast<std::streamsize>(sz * n));
            return out->good() ? sz * n : 0;
        }

        struct ProgressCtx {
            Progress* cb;
        };

        int OnProgress(void* ud, curl_off_t total, curl_off_t now, curl_off_t, curl_off_t) {
            auto* ctx = static_cast<ProgressCtx*>(ud);
            if (ctx->cb && *ctx->cb) {
                (*ctx->cb)(static_cast<std::uint64_t>(now), static_cast<std::uint64_t>(total));
            }
            return 0;
        }
    }

    std::string ObjectUrl(const std::string& bucket, const std::string& object) {
        std::string obj = object;
        while(!obj.empty() && obj.front() == '/') obj.erase(0, 1);
        return std::string(kHost) + "/" + bucket + "/" + obj;
    }

    std::string GetText(const std::string& url) {
        EnsureCurl();
        Easy c;
        std::string body;
        curl_easy_setopt(c.h, CURLOPT_URL, url.c_str());
        curl_easy_setopt(c.h, CURLOPT_WRITEFUNCTION, WriteToString);
        curl_easy_setopt(c.h, CURLOPT_WRITEDATA, &body);
        c.Perform("GET");
        return body;
    }

    void Download(const std::string& url, const std::filesystem::path& dest, Progress progress) {
        EnsureCurl();
        if (dest.has_parent_path()) std::filesystem::create_directories(dest.parent_path());
    
        auto part = dest;
        part += ".part";
    
        try {
            std::ofstream out(part, std::ios::binary | std::ios::trunc);
            if (!out) throw Error("Nelze otevřít pro zápis: " + part.string());
    
            Easy c;
            ProgressCtx ctx{&progress};
            curl_easy_setopt(c.h, CURLOPT_URL, url.c_str());
            curl_easy_setopt(c.h, CURLOPT_WRITEFUNCTION, WriteToFile);
            curl_easy_setopt(c.h, CURLOPT_WRITEDATA, &out);
            curl_easy_setopt(c.h, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(c.h, CURLOPT_XFERINFOFUNCTION, OnProgress);
            curl_easy_setopt(c.h, CURLOPT_XFERINFODATA, &ctx);
            c.Perform("download");
        } catch (...) {
            std::error_code ec;
            std::filesystem::remove(part, ec);
            throw;
        }
    
        std::error_code ec;
        std::filesystem::remove(dest, ec);
        std::filesystem::rename(part, dest);
    }
}
