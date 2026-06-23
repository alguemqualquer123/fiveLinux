#include "updater.h"
#include "installer.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

namespace fml {

std::string FiveMUpdater::getCurrentVersion(const std::string& installDir) const {
    auto iniPath = std::filesystem::path(installDir) / "CitizenFX.ini";
    if (!std::filesystem::exists(iniPath)) return "";

    std::ifstream ini(iniPath);
    if (!ini.is_open()) return "";

    std::string line;
    while (std::getline(ini, line)) {
        if (line.find("Version=") == 0) {
            return line.substr(8);
        }
    }
    return "";
}

std::string FiveMUpdater::getLatestVersion() const {
    std::string json = fetchRemoteBuildInfo();
    return parseVersionFromJson(json);
}

FiveMUpdateInfo FiveMUpdater::checkUpdate(const std::string& installDir) const {
    FiveMUpdateInfo info{};
    info.currentVersion = getCurrentVersion(installDir);
    info.latestVersion = getLatestVersion();

    info.updateAvailable = !info.currentVersion.empty() &&
                           !info.latestVersion.empty() &&
                           info.currentVersion != info.latestVersion;

    if (info.updateAvailable) {
        std::string json = fetchRemoteBuildInfo();
        info.downloadUrl = parseUrlFromJson(json);
    }

    return info;
}

bool FiveMUpdater::update(const std::string& installDir, const std::string& winePath,
                           ProgressCallback progress) {
    auto info = checkUpdate(installDir);
    if (!info.updateAvailable) {
        if (progress) progress(1.0f, "Already up to date!");
        return true;
    }

    if (progress) progress(0.0f, "Starting update...");

    std::string archivePath = installDir + "/fivem-update.7z";

    if (progress) progress(0.1f, "Downloading update...");
    std::string cmd = "curl -L -o \"" + archivePath + "\" '" + info.downloadUrl + "' 2>/dev/null";
    int result = std::system(cmd.c_str());
    if (result != 0) return false;

    if (progress) progress(0.6f, "Extracting update...");
    cmd = "cd \"" + installDir + "\" && 7z x \"" + archivePath + "\" -y >/dev/null 2>&1";
    result = std::system(cmd.c_str());

    std::error_code ec;
    std::filesystem::remove(archivePath, ec);

    if (progress) progress(1.0f, "Update complete!");
    return result == 0;
}

std::string FiveMUpdater::fetchRemoteBuildInfo() const {
    auto pipe = popen(
        "curl -sL 'https://runtime.fivem.net/artifacts/fivem/build_proton_linux/master/' 2>/dev/null",
        "r");
    if (!pipe) return "";

    std::ostringstream result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result << buffer;
    }
    pclose(pipe);
    return result.str();
}

std::string FiveMUpdater::parseVersionFromJson(const std::string& json) const {
    size_t pos = json.find("\"version\"");
    if (pos == std::string::npos) return "";
    size_t colon = json.find(':', pos);
    size_t q1 = json.find('"', colon + 1);
    size_t q2 = json.find('"', q1 + 1);
    if (q1 != std::string::npos && q2 != std::string::npos) {
        return json.substr(q1 + 1, q2 - q1 - 1);
    }
    return "";
}

std::string FiveMUpdater::parseUrlFromJson(const std::string& json) const {
    size_t pos = json.find("\"file\"");
    if (pos == std::string::npos) pos = json.find("\"url\"");
    if (pos == std::string::npos) return "";
    size_t colon = json.find(':', pos);
    size_t q1 = json.find('"', colon + 1);
    size_t q2 = json.find('"', q1 + 1);
    if (q1 != std::string::npos && q2 != std::string::npos) {
        return json.substr(q1 + 1, q2 - q1 - 1);
    }
    return "";
}

} // namespace fml
