#include "installation_detector.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace fml {

std::vector<std::string> GtaInstallationDetector::getSteamPaths() const {
    std::vector<std::string> paths;
    const char* home = getenv("HOME");
    if (!home) return paths;

    std::string steamRoot = std::string(home) + "/.steam/steam";
    paths.push_back(steamRoot + "/steamapps/common/Grand Theft Auto V");
    paths.push_back(steamRoot + "/steamapps/common/Grand Theft Auto V/");

    std::string libraryfolders = steamRoot + "/steamapps/libraryfolders.vdf";
    if (std::filesystem::exists(libraryfolders)) {
        std::ifstream vdf(libraryfolders);
        std::string line;
        while (std::getline(vdf, line)) {
            size_t pos = line.find("\"path\"");
            if (pos != std::string::npos) {
                auto start = line.find('"', pos + 6);
                auto end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string libPath = line.substr(start + 1, end - start - 1);
                    paths.push_back(libPath + "/steamapps/common/Grand Theft Auto V");
                }
            }
        }
    }

    return paths;
}

std::vector<std::string> GtaInstallationDetector::getEpicPaths() const {
    std::vector<std::string> paths;
    const char* home = getenv("HOME");
    if (!home) return paths;

    paths.push_back(std::string(home) + "/Games/GTA V");
    paths.push_back(std::string(home) + "/Epic Games/GTA V");
    paths.push_back(std::string(home) + "/.local/share/Steam/steamapps/common/Grand Theft Auto V");

    return paths;
}

std::vector<std::string> GtaInstallationDetector::getRockstarPaths() const {
    std::vector<std::string> paths;
    const char* home = getenv("HOME");
    if (!home) return paths;

    paths.push_back(std::string(home) + "/Games/Rockstar Games/Grand Theft Auto V");
    paths.push_back("/opt/Rockstar Games/Grand Theft Auto V");

    return paths;
}

bool GtaInstallationDetector::validateGtaDirectory(const std::string& path) const {
    if (!std::filesystem::exists(path)) return false;

    auto exePath = std::filesystem::path(path) / "GTA5.exe";
    if (!std::filesystem::exists(exePath)) return false;

    bool hasX64 = false;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        if (name.find("x64") == 0 && name.find(".rpf") != std::string::npos) {
            hasX64 = true;
            break;
        }
    }

    return hasX64;
}

std::string GtaInstallationDetector::detectVersion(const std::string& path) const {
    auto exePath = std::filesystem::path(path) / "GTA5.exe";
    if (!std::filesystem::exists(exePath)) return "unknown";

    std::string cmd = "stat -c %Y \"" + exePath.string() + "\" 2>/dev/null";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "unknown";

    char buffer[64];
    std::string mtime;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        mtime = buffer;
        mtime.erase(std::remove(mtime.begin(), mtime.end(), '\n'), mtime.end());
    }
    pclose(pipe);

    return "mtime:" + mtime;
}

InstallSource GtaInstallationDetector::getSource(const std::string& path) const {
    if (path.find(".steam") != std::string::npos ||
        path.find("steamapps") != std::string::npos) {
        return InstallSource::Steam;
    }
    if (path.find("Epic") != std::string::npos ||
        path.find("epic") != std::string::npos) {
        return InstallSource::EpicGames;
    }
    if (path.find("Rockstar") != std::string::npos ||
        path.find("rockstar") != std::string::npos) {
        return InstallSource::RockstarLauncher;
    }
    return InstallSource::Manual;
}

std::optional<GtaInstallation> GtaInstallationDetector::detectGta(
    const std::string& installDir) const
{
    if (validateGtaDirectory(installDir)) {
        GtaInstallation info{};
        info.path = installDir;
        info.source = getSource(installDir);
        info.version = detectVersion(installDir);
        info.valid = true;

        uint64_t totalSize = 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(installDir,
                std::filesystem::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file()) {
                std::error_code ec;
                totalSize += entry.file_size(ec);
            }
        }
        info.totalSizeBytes = totalSize;
        return info;
    }

    return std::nullopt;
}

std::vector<GtaInstallation> GtaInstallationDetector::findInstallations() const {
    std::vector<GtaInstallation> results;
    std::vector<std::string> allPaths;

    auto steamPaths = getSteamPaths();
    allPaths.insert(allPaths.end(), steamPaths.begin(), steamPaths.end());

    auto epicPaths = getEpicPaths();
    allPaths.insert(allPaths.end(), epicPaths.begin(), epicPaths.end());

    auto rockstarPaths = getRockstarPaths();
    allPaths.insert(allPaths.end(), rockstarPaths.begin(), rockstarPaths.end());

    for (auto& path : allPaths) {
        if (auto install = detectGta(path)) {
            results.push_back(*install);
        }
    }

    return results;
}

std::optional<GtaInstallation> GtaInstallationDetector::getInstallationInfo(
    const std::string& path) const
{
    return detectGta(path);
}

} // namespace fml
