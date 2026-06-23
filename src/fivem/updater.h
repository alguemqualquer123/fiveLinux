#pragma once

#include <string>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

struct FiveMUpdateInfo {
    bool updateAvailable;
    std::string currentVersion;
    std::string latestVersion;
    std::string downloadUrl;
    uint64_t downloadSize;
};

class FiveMUpdater {
public:
    FiveMUpdater() = default;

    FiveMUpdateInfo checkUpdate(const std::string& installDir) const;
    bool update(const std::string& installDir, const std::string& winePath,
                ProgressCallback progress = nullptr);
    std::string getCurrentVersion(const std::string& installDir) const;
    std::string getLatestVersion() const;

private:
    std::string fetchRemoteBuildInfo() const;
    std::string parseVersionFromJson(const std::string& json) const;
    std::string parseUrlFromJson(const std::string& json) const;
};

} // namespace fml
