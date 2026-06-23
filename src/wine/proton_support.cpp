#include "proton_support.h"
#include "fivemlinux/types.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace fml {

std::vector<SteamLibrary> ProtonSupport::getSteamLibraries() const {
    return libraries_;
}

bool ProtonSupport::detectProton() {
    if (detected_) return true;

    const char* home = getenv("HOME");
    if (!home) return false;

    std::string steamRoot = std::string(home) + "/.steam/steam";
    std::string defaultLib = steamRoot + "/steamapps";

    libraries_.clear();

    SteamLibrary defaultLibSteam{};
    defaultLibSteam.path = defaultLib;
    defaultLibSteam.valid = std::filesystem::exists(defaultLib);

    if (defaultLibSteam.valid) {
        std::string libraryfolders = steamRoot + "/steamapps/libraryfolders.vdf";
        if (std::filesystem::exists(libraryfolders)) {
            std::ifstream vdf(libraryfolders);
            if (vdf.is_open()) {
                std::string line;
                while (std::getline(vdf, line)) {
                    size_t pathPos = line.find("\"path\"");
                    if (pathPos != std::string::npos) {
                        auto start = line.find('"', pathPos + 6);
                        auto end = line.find('"', start + 1);
                        if (start != std::string::npos && end != std::string::npos) {
                            std::string libPath = line.substr(start + 1, end - start - 1);
                            libPath += "/steamapps";
                            if (std::filesystem::exists(libPath)) {
                                SteamLibrary lib{};
                                lib.path = libPath;
                                lib.valid = true;
                                lib.protonVersions = scanProtonInDir(libPath);
                                if (!lib.protonVersions.empty()) {
                                    libraries_.push_back(lib);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    libraries_.insert(libraries_.begin(), defaultLibSteam);
    for (auto& lib : libraries_) {
        lib.protonVersions = scanProtonInDir(lib.path);
    }

    detected_ = true;
    return !libraries_.empty();
}

std::vector<ProtonVersion> ProtonSupport::scanProtonInDir(const std::string& steamappsPath) {
    std::vector<ProtonVersion> versions;
    std::string commonDir = steamappsPath + "/common";

    if (!std::filesystem::exists(commonDir)) return versions;

    for (auto& entry : std::filesystem::directory_iterator(commonDir)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();

        bool isProton = false;
        WineType type = WineType::Unknown;

        if (name.find("Proton") == 0) {
            isProton = true;
            if (name.find("Experimental") != std::string::npos) {
                type = WineType::ProtonExperimental;
            } else if (name.find("Hotfix") != std::string::npos) {
                type = WineType::ProtonHotfix;
            } else {
                type = WineType::Proton;
            }
        } else if (name.find("Steam Linux Runtime") != std::string::npos) {
            isProton = true;
            type = WineType::Proton;
        } else if (name.find("GE-Proton") != std::string::npos || name.find("Proton-GE") != std::string::npos) {
            isProton = true;
            type = WineType::WineGE;
        }

        if (!isProton) continue;

        ProtonVersion ver{};
        ver.name = name;
        ver.path = entry.path().string();
        ver.type = type;
        ver.available = true;

        std::string manifestPath = ver.path + "/toolmanifest.vdf";
        ver.version = parseToolManifest(ver.path);

        versions.push_back(ver);
    }

    return versions;
}

std::string ProtonSupport::parseToolManifest(const std::string& protonPath) {
    std::string manifestPath = protonPath + "/toolmanifest.vdf";
    if (!std::filesystem::exists(manifestPath)) return "unknown";

    std::ifstream file(manifestPath);
    if (!file.is_open()) return "unknown";

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find("\"version\"");
        if (pos != std::string::npos) {
            auto start = line.find('"', pos + 9);
            auto end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                return line.substr(start + 1, end - start - 1);
            }
        }
    }

    return "unknown";
}

std::vector<ProtonVersion> ProtonSupport::getProtonVersions() const {
    std::vector<ProtonVersion> all;
    for (auto& lib : libraries_) {
        for (auto& ver : lib.protonVersions) {
            all.push_back(ver);
        }
    }
    return all;
}

std::optional<ProtonVersion> ProtonSupport::getProtonPath(const std::string& version) const {
    for (auto& lib : libraries_) {
        for (auto& ver : lib.protonVersions) {
            if (ver.name.find(version) != std::string::npos) {
                return ver;
            }
        }
    }
    return std::nullopt;
}

bool ProtonSupport::isProtonAvailable() const {
    return detected_ && !libraries_.empty() &&
           std::any_of(libraries_.begin(), libraries_.end(),
               [](const SteamLibrary& lib) { return !lib.protonVersions.empty(); });
}

std::optional<ProtonVersion> ProtonSupport::getRecommendedProton(
    const std::string& /*gameId*/) const
{
    for (auto& lib : libraries_) {
        for (auto& ver : lib.protonVersions) {
            if (ver.type == WineType::ProtonExperimental) return ver;
        }
    }
    for (auto& lib : libraries_) {
        for (auto& ver : lib.protonVersions) {
            if (ver.type == WineType::Proton) return ver;
        }
    }
    for (auto& lib : libraries_) {
        if (!lib.protonVersions.empty()) return lib.protonVersions.front();
    }
    return std::nullopt;
}

std::optional<WineInfo> ProtonSupport::getWineInfoFromProton(const ProtonVersion& proton) const {
    if (!proton.available) return std::nullopt;

    WineInfo info{};
    info.type = proton.type;
    info.version = proton.version;
    info.path = proton.path + "/files/bin/wine";
    info.is64bit = std::filesystem::exists(proton.path + "/files/lib64");
    info.proton_version = proton.version;

    if (std::filesystem::exists(info.path)) {
        return info;
    }

    return std::nullopt;
}

} // namespace fml
