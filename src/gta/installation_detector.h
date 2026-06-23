#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

#include "fivemlinux/types.h"

namespace fml {

struct GtaInstallation {
    std::string path;
    InstallSource source;
    std::string version;
    bool valid;
    uint64_t totalSizeBytes;
};

class GtaInstallationDetector {
public:
    GtaInstallationDetector() = default;

    std::optional<GtaInstallation> detectGta(const std::string& installDir) const;
    std::vector<GtaInstallation> findInstallations() const;
    std::optional<GtaInstallation> getInstallationInfo(const std::string& path) const;
    InstallSource getSource(const std::string& path) const;

private:
    std::vector<std::string> getSteamPaths() const;
    std::vector<std::string> getEpicPaths() const;
    std::vector<std::string> getRockstarPaths() const;
    bool validateGtaDirectory(const std::string& path) const;
    std::string detectVersion(const std::string& path) const;
};

} // namespace fml
