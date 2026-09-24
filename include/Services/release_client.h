#pragma once
#include <string>

#include "Models/release.h"

namespace Services {
    // Find latest release for channel
    Models::Release LatestRelease(const std::string& channel = "stable");
}
