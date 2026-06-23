#include "prefix_manager.h"
#include "fivemlinux/types.h"

#include <cstdlib>
#include <array>
#include <memory>
#include <stdexcept>
#include <sstream>

namespace fml {

bool WinePrefixManager::prefixExists(const std::string& path) const {
    return std::filesystem::exists(path) &&
           std::filesystem::exists(std::filesystem::path(path) / "drive_c");
}

std::optional<PrefixInfo> WinePrefixManager::getPrefixInfo(const std::string& path) {
    PrefixInfo info{};
    info.path = path;
    info.exists = prefixExists(path);

    if (!info.exists) return info;

    auto driveC = std::filesystem::path(path) / "drive_c";
    uint64_t totalSize = 0;
    for (auto& entry : std::filesystem::recursive_directory_iterator(driveC,
            std::filesystem::directory_options::skip_permission_denied)) {
        if (entry.is_regular_file()) {
            totalSize += entry.file_size();
        }
    }
    info.sizeBytes = totalSize;
    info.is64bit = std::filesystem::exists(std::filesystem::path(path) / "drive_c" / "windows" / "syswow64");

    return info;
}

std::optional<PrefixInfo> WinePrefixManager::createPrefix(
    const std::string& path,
    const std::string& winePath,
    bool is64bit)
{
    if (prefixExists(path)) {
        return getPrefixInfo(path);
    }

    if (!createDirectoryStructure(path, is64bit)) {
        return std::nullopt;
    }

    if (!initializeRegistry(path, is64bit)) {
        return std::nullopt;
    }

    if (!runWineboot(winePath, path)) {
        return std::nullopt;
    }

    return getPrefixInfo(path);
}

bool WinePrefixManager::deletePrefix(const std::string& path) {
    if (!prefixExists(path)) return false;
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    return !ec;
}

bool WinePrefixManager::backupPrefix(const std::string& path, const std::string& backupPath) {
    if (!prefixExists(path)) return false;

    auto parentDir = std::filesystem::path(backupPath).parent_path();
    if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
        std::filesystem::create_directories(parentDir);
    }

    std::string cmd = "tar -czf \"" + backupPath + "\" -C \"" +
                      std::filesystem::path(path).parent_path().string() +
                      "\" \"" + std::filesystem::path(path).filename().string() + "\" 2>/dev/null";
    int result = std::system(cmd.c_str());
    return result == 0;
}

bool WinePrefixManager::restorePrefix(const std::string& backupPath, const std::string& targetPath) {
    if (!std::filesystem::exists(backupPath)) return false;

    auto parentDir = std::filesystem::path(targetPath).parent_path();
    if (!std::filesystem::exists(parentDir)) {
        std::filesystem::create_directories(parentDir);
    }

    std::string cmd = "tar -xzf \"" + backupPath + "\" -C \"" +
                      parentDir.string() + "\" 2>/dev/null";
    int result = std::system(cmd.c_str());
    return result == 0;
}

bool WinePrefixManager::repairPrefix(const std::string& path) {
    if (!prefixExists(path)) return false;

    auto systemReg = std::filesystem::path(path) / "system.reg";
    auto userReg = std::filesystem::path(path) / "user.reg";

    if (std::filesystem::exists(systemReg)) {
        std::error_code ec;
        std::filesystem::remove(systemReg, ec);
    }
    if (std::filesystem::exists(userReg)) {
        std::error_code ec;
        std::filesystem::remove(userReg, ec);
    }

    return true;
}

bool WinePrefixManager::createDirectoryStructure(const std::string& path, bool is64bit) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) return false;

    auto driveC = std::filesystem::path(path) / "drive_c";
    std::filesystem::create_directories(driveC / "windows" / "system32", ec);
    if (ec) return false;

    if (is64bit) {
        std::filesystem::create_directories(driveC / "windows" / "syswow64", ec);
        if (ec) return false;
    }

    std::filesystem::create_directories(driveC / "windows" / "fonts", ec);
    std::filesystem::create_directories(driveC / "windows" / "temp", ec);
    std::filesystem::create_directories(driveC / "users" / "steamuser" / "AppData" / "Local" / "Temp", ec);
    std::filesystem::create_directories(driveC / "Program Files", ec);

    return !ec;
}

bool WinePrefixManager::initializeRegistry(const std::string& prefixPath, bool is64bit) {
    auto regDir = std::filesystem::path(prefixPath);
    std::ofstream systemReg(regDir / "system.reg");
    if (!systemReg.is_open()) return false;

    systemReg << "WINE REGISTRY Version 2\n";
    systemReg << ";; All keys relative to \\\\Machine\n\n";
    systemReg.close();

    std::ofstream userReg(regDir / "user.reg");
    if (!userReg.is_open()) return false;

    userReg << "WINE REGISTRY Version 2\n";
    userReg << ";; All keys relative to \\\\User\\\\CurrentUser\n\n";
    userReg.close();

    return true;
}

bool WinePrefixManager::runWineboot(const std::string& winePath, const std::string& prefixPath) {
    return runWineCommand(winePath, prefixPath, "wineboot -u");
}

bool WinePrefixManager::runWineCommand(
    const std::string& winePath,
    const std::string& prefixPath,
    const std::string& args)
{
    std::string cmd = "WINEPREFIX=\"" + prefixPath + "\" \"" + winePath + "\" " + args + " 2>&1";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {}

    int status = pclose(pipe);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

} // namespace fml
