#pragma once
#include <string>

namespace Models {
    struct Release {
        std::string version;
        std::string archive;
        std::string sha256;
        std::string url;
    };
}
