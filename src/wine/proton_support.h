#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

#include "fivemlinux/types.h"

namespace fml {

struct ProtonVersion {
    std::string name;
    std::string path;
    std::string appid;
    std::string version;
    WineType type;
    bool available;
};

struct SteamLibrary {
    std::string path;
    bool valid;
    std::vector<ProtonVersion> protonVersions;
};

class ProtonSupport {
public:
    ProtonSupport() = default;

    bool detectProton();
    std::vector<ProtonVersion> getProtonVersions() const;
    std::optional<ProtonVersion> getProtonPath(const std::string& version) const;
    bool isProtonAvailable() const;
    std::optional<ProtonVersion> getRecommendedProton(const std::string& gameId = "") const;
    std::vector<SteamLibrary> getSteamLibraries() const;
    std::optional<WineInfo> getWineInfoFromProton(const ProtonVersion& proton) const;

private:
    std::vector<SteamLibrary> scanSteamLibraries();
    std::vector<ProtonVersion> scanProtonInDir(const std::string& steamappsPath);
    std::string parseToolManifest(const std::string& protonPath);
    bool parseCompatibilityToolsVdf(const std::string& vdfPath,
                                     std::map<std::string, std::string>& tools);

    std::vector<SteamLibrary> libraries_;
    bool detected_ = false;
};

} // namespace fml
