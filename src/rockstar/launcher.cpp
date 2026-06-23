#include "launcher.h"
#include "fivemlinux/types.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace fml {

std::vector<std::string> RockstarLauncher::getSearchPaths() {
    const char* home = getenv("HOME");
    if (!home) return {};

    std::string h(home);
    return {
        h + "/Games/Rockstar Games/Launcher",
        h + "/.local/share/Rockstar Games/Launcher",
        h + "/.wine/drive_c/Program Files/Rockstar Games/Launcher",
        h + "/.wine/drive_c/Program Files (x86)/Rockstar Games/Launcher",
        "/opt/Rockstar Games/Launcher",
        "/usr/local/share/Rockstar Games/Launcher"
    };
}

std::string RockstarLauncher::getSocialClubRegistryPath() {
    return "HKEY_CURRENT_USER\\Software\\Rockstar Games\\Social Club";
}

std::string RockstarLauncher::getDefaultPrefix() {
    const char* home = getenv("HOME");
    return home ? std::string(home) + "/.fivem-linux/prefix" : "";
}

std::string RockstarLauncher::getLauncherPath() const {
    auto paths = getSearchPaths();
    for (auto& path : paths) {
        auto launcher = std::filesystem::path(path) / "RockstarLauncher.exe";
        if (std::filesystem::exists(launcher)) {
            return launcher.string();
        }
    }
    return "";
}

RockstarInstallInfo RockstarLauncher::detect() const {
    RockstarInstallInfo info{};
    info.installed = false;
    info.status = RockstarAuthStatus::NotInstalled;

    auto paths = getSearchPaths();
    for (auto& path : paths) {
        auto launcher = std::filesystem::path(path) / "RockstarLauncher.exe";
        if (std::filesystem::exists(launcher)) {
            info.installed = true;
            info.installPath = path;
            info.launcherPath = launcher.string();
            info.status = RockstarAuthStatus::Installed;

            uint64_t totalSize = 0;
            for (auto& entry : std::filesystem::recursive_directory_iterator(path,
                    std::filesystem::directory_options::skip_permission_denied)) {
                if (entry.is_regular_file()) {
                    std::error_code ec;
                    totalSize += entry.file_size(ec);
                }
            }
            info.totalSizeBytes = totalSize;
            break;
        }
    }

    return info;
}

bool RockstarLauncher::isInstalled() const {
    return detect().installed;
}

RockstarLauncherState RockstarLauncher::getState() const {
    RockstarLauncherState state{};
    state.authStatus = RockstarAuthStatus::NotInstalled;
    state.launcherRunning = false;
    state.socialClubRunning = false;
    state.cloudSyncEnabled = true;

    auto info = detect();
    if (info.installed) {
        state.authStatus = info.status;
        state.installDir = info.installPath;
        state.prefixPath = getDefaultPrefix();
    }

    state.launcherRunning = isRunning();

    return state;
}

bool RockstarLauncher::launch(const std::string& winePath) {
    if (isRunning()) return false;

    std::string launcher = getLauncherPath();
    if (launcher.empty()) return false;

    std::string prefixPath = getDefaultPrefix();
    if (!setupWineEnvironment(prefixPath)) return false;

    pid_ = fork();
    if (pid_ == -1) return false;

    if (pid_ == 0) {
        std::string cmd = "\"" + winePath + "\" \"" + launcher + "\"";
        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        _exit(1);
    }

    return true;
}

bool RockstarLauncher::stop() {
    if (pid_ <= 0) return false;

    if (isRunning()) {
        kill(pid_, SIGTERM);
        usleep(2000000);
        if (isRunning()) {
            kill(pid_, SIGKILL);
        }
    }

    pid_ = 0;
    return true;
}

bool RockstarLauncher::isRunning() const {
    if (pid_ <= 0) return false;
    int status;
    pid_t result = waitpid(pid_, &status, WNOHANG);
    return result == 0;
}

bool RockstarLauncher::setupWineEnvironment(const std::string& prefixPath) {
    setenv("WINEPREFIX", prefixPath.c_str(), 1);
    setenv("WINEDEBUG", "-all", 1);
    setenv("WINEDLLOVERRIDES", "winemenubuilder.exe=d", 1);
    return true;
}

bool RockstarLauncher::installDependencies(const std::string& prefixPath) {
    std::vector<std::string> deps = {
        "winetricks -q vcrun2019",
        "winetricks -q d3dcompiler_47",
        "winetricks -q d3dx9",
        "winetricks -q d3dx11_43",
        "winetricks -q d3dx10_43",
        "winetricks -q xact",
        "winetricks -q xact_x64",
        "winetricks -q fontsmooth=rgb"
    };

    for (auto& dep : deps) {
        std::string cmd = "WINEPREFIX=\"" + prefixPath + "\" " + dep + " 2>/dev/null";
        std::system(cmd.c_str());
    }

    return true;
}

bool RockstarLauncher::configurePrefix(const std::string& prefixPath) {
    std::error_code ec;
    std::filesystem::create_directories(prefixPath, ec);
    if (ec) return false;

    auto driveC = std::filesystem::path(prefixPath) / "drive_c";
    std::filesystem::create_directories(driveC / "Program Files" / "Rockstar Games" / "Launcher", ec);
    std::filesystem::create_directories(driveC / "Program Files" / "Rockstar Games" / "GTAV", ec);
    std::filesystem::create_directories(driveC / "Program Files (x86)" / "Rockstar Games" / "Social Club", ec);
    std::filesystem::create_directories(driveC / "Users" / "steamuser" / "Documents" / "Rockstar Games" / "Social Club", ec);

    return !ec;
}

bool RockstarLauncher::installSocialClub(const std::string& prefixPath) {
    std::string cmd =
        "WINEPREFIX=\"" + prefixPath + "\" "
        "wine reg add \"HKEY_CURRENT_USER\\Software\\Rockstar Games\\Social Club\" "
        "/v \"InstallPath\" /t REG_SZ /d \"C:\\Program Files (x86)\\Rockstar Games\\Social Club\" "
        "/f 2>/dev/null";
    return std::system(cmd.c_str()) == 0;
}

bool RockstarLauncher::installLauncher(const std::string& prefixPath) {
    return installSocialClub(prefixPath);
}

bool RockstarLauncher::checkSocialClubVersion(const std::string& /*prefixPath*/) {
    return true;
}

bool RockstarLauncher::login(const std::string& /*email*/, const std::string& /*password*/) {
    if (!isRunning()) {
        if (!launch()) return false;
    }
    return true;
}

bool RockstarLauncher::loginWithSteam() {
    if (!isRunning()) {
        if (!launch()) return false;
    }
    return true;
}

bool RockstarLauncher::logout() {
    return stop();
}

bool RockstarLauncher::isOnline() const {
    return isRunning();
}

bool RockstarLauncher::runRockstarCommand(const std::string& winePath,
                                           const std::string& prefixPath,
                                           const std::string& args) {
    std::string cmd = "WINEPREFIX=\"" + prefixPath + "\" \"" + winePath + "\" " + args + " 2>&1";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {}

    int status = pclose(pipe);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

} // namespace fml
