#include "installer.h"
#include "fivemlinux/types.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <array>

namespace fml {

bool FiveMInstaller::isInstalled(const std::string& installDir) const {
    auto exePath = std::filesystem::path(installDir) / "FiveM.exe";
    auto iniPath = std::filesystem::path(installDir) / "CitizenFX.ini";
    auto cacheDir = std::filesystem::path(installDir) / "cache";

    return std::filesystem::exists(exePath) &&
           (std::filesystem::exists(iniPath) || std::filesystem::exists(cacheDir));
}

FiveMInstallInfo FiveMInstaller::getInstallInfo(const std::string& installDir) const {
    FiveMInstallInfo info{};
    info.installPath = installDir;
    info.installed = isInstalled(installDir);

    auto exePath = std::filesystem::path(installDir) / "FiveM.exe";
    auto iniPath = std::filesystem::path(installDir) / "CitizenFX.ini";
    auto cacheDir = std::filesystem::path(installDir) / "cache";

    info.hasExe = std::filesystem::exists(exePath);
    info.hasCitizenFX = std::filesystem::exists(iniPath);
    info.hasCache = std::filesystem::exists(cacheDir);

    if (iniPath.exists()) {
        std::ifstream ini(iniPath);
        if (ini.is_open()) {
            std::string line;
            while (std::getline(ini, line)) {
                if (line.find("Version=") == 0) {
                    info.version = line.substr(8);
                }
            }
        }
    }

    if (info.installed) {
        uint64_t totalSize = 0;
        for (auto& entry : std::filesystem::recursive_directory_iterator(installDir,
                std::filesystem::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }
        info.totalSizeBytes = totalSize;
        info.status = FiveMStatus::Installed;
    } else if (info.hasExe) {
        info.status = FiveMStatus::MissingDependencies;
    } else {
        info.status = FiveMStatus::NotInstalled;
    }

    return info;
}

bool FiveMInstaller::install(const std::string& installDir, const std::string& winePath,
                              ProgressCallback progress) {
    if (progress) progress(0.0f, "Starting FiveM installation...");

    if (!std::filesystem::exists(installDir)) {
        std::filesystem::create_directories(installDir);
    }

    if (progress) progress(0.1f, "Fetching build information...");
    std::string buildInfo = fetchBuildInfo();
    if (buildInfo.empty()) return false;

    std::string downloadUrl = extractUrlFromJson(buildInfo);
    if (downloadUrl.empty()) return false;

    if (progress) progress(0.2f, "Downloading FiveM...");
    std::string archivePath = installDir + "/fivem-update.7z";
    if (!downloadFile(downloadUrl, archivePath, progress)) return false;

    if (progress) progress(0.7f, "Extracting FiveM...");
    if (!extractArchive(archivePath, installDir)) return false;

    if (progress) progress(0.9f, "Creating cache structure...");
    if (!createCacheStructure(installDir)) return false;

    std::error_code ec;
    std::filesystem::remove(archivePath, ec);

    if (progress) progress(1.0f, "Installation complete!");
    return true;
}

bool FiveMInstaller::uninstall(const std::string& installDir) {
    if (!std::filesystem::exists(installDir)) return false;
    std::error_code ec;
    std::filesystem::remove_all(installDir, ec);
    return !ec;
}

std::string FiveMInstaller::fetchBuildInfo() {
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

std::string FiveMInstaller::extractUrlFromJson(const std::string& json) {
    size_t pos = json.find("\"file\"");
    if (pos == std::string::npos) {
        pos = json.find("\"url\"");
    }
    if (pos == std::string::npos) return "";

    size_t colon = json.find(':', pos);
    size_t quote1 = json.find('"', colon + 1);
    size_t quote2 = json.find('"', quote1 + 1);

    if (quote1 != std::string::npos && quote2 != std::string::npos) {
        return json.substr(quote1 + 1, quote2 - quote1 - 1);
    }
    return "";
}

bool FiveMInstaller::downloadFile(const std::string& url, const std::string& outputPath,
                                   ProgressCallback progress) {
    std::string cmd = "curl -L -o \"" + outputPath + "\" '" + url + "' 2>/dev/null";
    int result = std::system(cmd.c_str());
    return result == 0 && std::filesystem::exists(outputPath);
}

bool FiveMInstaller::extractArchive(const std::string& archivePath, const std::string& targetDir) {
    std::string cmd = "cd \"" + targetDir + "\" && 7z x \"" + archivePath + "\" -y >/dev/null 2>&1";
    int result = std::system(cmd.c_str());
    return result == 0;
}

bool FiveMInstaller::createCacheStructure(const std::string& installDir) {
    std::error_code ec;
    std::filesystem::create_directories(installDir + "/cache/game", ec);
    std::filesystem::create_directories(installDir + "/cache/priv", ec);
    std::filesystem::create_directories(installDir + "/cache/files", ec);
    std::filesystem::create_directories(installDir + "/cache/server-cache-priv", ec);
    return !ec;
}

} // namespace fml
