#pragma once
#include <string>

#include "Models/release.h"

namespace Services {
    // Read releases_windows.json and find new release struct channel
    // Remove API::Error, if downloaded or structure JSON crashing
    Models::Release LatestRelease(const std::string& channel = "stable");
}
