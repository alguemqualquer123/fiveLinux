#include "launcher.h"
#include "fivemlinux/types.h"

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>

namespace fml {

void FiveMLauncher::setupEnvironment(const std::string& prefixPath) {
    setenv("WINEPREFIX", prefixPath.c_str(), 1);
    setenv("WINEDEBUG", "-all", 1);
    setenv("DXVK_HUD", "fps,version,compiler", 1);
    setenv("WINE_FULLSCREEN_FSR", "1", 1);
    setenv("WINE_FULLSCREEN_FSR_STRENGTH", "5", 1);
}

bool FiveMLauncher::startProcess(const std::string& winePath, const std::string& exePath,
                                  const std::string& prefixPath, const std::string& args) {
    setupEnvironment(prefixPath);

    pid_ = fork();
    if (pid_ == -1) return false;

    if (pid_ == 0) {
        std::string fullCmd = "\"" + winePath + "\" \"" + exePath + "\" " + args;
        execl("/bin/sh", "sh", "-c", fullCmd.c_str(), nullptr);
        _exit(1);
    }

    return true;
}

bool FiveMLauncher::launch(const std::string& installDir, const std::string& winePath,
                            const std::string& serverUrl) {
    if (isRunning()) return false;

    auto exePath = std::filesystem::path(installDir) / "FiveM.exe";
    if (!std::filesystem::exists(exePath)) return false;

    std::string prefixPath;
    auto iniPath = std::filesystem::path(installDir) / "CitizenFX.ini";
    if (std::filesystem::exists(iniPath)) {
        std::ifstream ini(iniPath);
        std::string line;
        while (std::getline(ini, line)) {
            if (line.find("PrefixPath=") == 0) {
                prefixPath = line.substr(11);
            }
        }
    }
    if (prefixPath.empty()) {
        prefixPath = std::filesystem::path(installDir).parent_path().string() + "/prefix";
    }

    std::string args;
    if (!serverUrl.empty()) {
        args = "-connect " + serverUrl;
    }

    return startProcess(winePath, exePath.string(), prefixPath, args);
}

bool FiveMLauncher::isRunning() const {
    if (pid_ <= 0) return false;

    int status;
    pid_t result = waitpid(pid_, &status, WNOHANG);
    return result == 0;
}

bool FiveMLauncher::stop() {
    if (pid_ <= 0) return false;

    if (isRunning()) {
        kill(pid_, SIGTERM);
        usleep(500000);
        if (isRunning()) {
            kill(pid_, SIGKILL);
        }
    }

    pid_ = 0;
    return true;
}

pid_t FiveMLauncher::getProcessId() const {
    return pid_;
}

} // namespace fml
