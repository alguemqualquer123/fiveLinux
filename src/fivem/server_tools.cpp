#include "server_tools.h"
#include "fivemlinux/types.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace fml {

bool FiveMServerTools::installServer(const std::string& installDir,
                                      ProgressCallback progress) {
    if (!std::filesystem::exists(installDir)) {
        std::filesystem::create_directories(installDir);
    }

    if (progress) progress(0.0f, "Downloading server artifacts...");
    if (!downloadServerArtifacts(installDir, progress)) return false;

    if (progress) progress(0.7f, "Extracting server...");
    std::string archive = installDir + "/server.7z";
    if (!extractServer(archive, installDir)) return false;

    std::error_code ec;
    std::filesystem::remove(archive, ec);

    if (progress) progress(1.0f, "Server installed!");
    return true;
}

bool FiveMServerTools::updateServer(const std::string& installDir,
                                     ProgressCallback progress) {
    return installServer(installDir, progress);
}

bool FiveMServerTools::downloadServerArtifacts(const std::string& targetDir,
                                                ProgressCallback progress) {
    if (progress) progress(0.1f, "Fetching latest server build...");

    std::string cmd =
        "curl -sL 'https://runtime.fivem.net/artifacts/fivem/build_proton_linux/master/' "
        "| grep -o '\"file\":\"[^\"]*server[^\"]*\"' | head -1 | cut -d'\"' -f4 > /tmp/fivem_server_url.txt 2>/dev/null";
    std::system(cmd.c_str());

    std::ifstream urlFile("/tmp/fivem_server_url.txt");
    std::string serverUrl;
    if (urlFile.is_open()) {
        std::getline(urlFile, serverUrl);
        urlFile.close();
    }

    std::system("rm -f /tmp/fivem_server_url.txt");

    if (serverUrl.empty()) return false;

    if (serverUrl.substr(0, 4) != "http") {
        serverUrl = "https://runtime.fivem.net" + serverUrl;
    }

    if (progress) progress(0.3f, "Downloading server package...");
    std::string dlCmd = "curl -L -o \"" + targetDir + "/server.7z\" '" + serverUrl + "' 2>/dev/null";
    int result = std::system(dlCmd.c_str());
    return result == 0;
}

bool FiveMServerTools::extractServer(const std::string& archivePath,
                                      const std::string& targetDir) {
    std::string cmd = "cd \"" + targetDir + "\" && 7z x \"" + archivePath + "\" -y >/dev/null 2>&1";
    int result = std::system(cmd.c_str());
    return result == 0;
}

pid_t FiveMServerTools::startServer(const std::string& serverDir, uint32_t port) {
    auto fxserverPath = std::filesystem::path(serverDir) / "FXServer" / "server" / "Bin" / "fxserver.exe";
    if (!std::filesystem::exists(fxserverPath)) {
        fxserverPath = std::filesystem::path(serverDir) / "server" / "Bin" / "fxserver.exe";
    }
    if (!std::filesystem::exists(fxserverPath)) return -1;

    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";

    pid_t pid = fork();
    if (pid == -1) return -1;

    if (pid == 0) {
        if (!prefixPath.empty()) setenv("WINEPREFIX", prefixPath.c_str(), 1);

        std::string portStr = std::to_string(port);
        std::string cmd = "wine \"" + fxserverPath.string() + "\" +exec server.cfg +port " + portStr;
        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        _exit(1);
    }

    return pid;
}

bool FiveMServerTools::stopServer(pid_t pid) {
    if (pid <= 0) return false;

    kill(pid, SIGTERM);
    usleep(2000000);

    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0) {
        kill(pid, SIGKILL);
    }
    return true;
}

ServerStatus FiveMServerTools::getServerStatus(pid_t pid) const {
    ServerStatus status{};
    status.pid = pid;

    if (pid <= 0) return status;

    int killResult = kill(pid, 0);
    status.running = (killResult == 0);

    if (status.running) {
        std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
        if (std::filesystem::exists(statusPath)) {
            std::ifstream statusFile(statusPath);
            std::string line;
            while (std::getline(statusFile, line)) {
                if (line.find("StartTime:") == 0) {
                    // Parse start time for uptime calculation
                }
            }
        }
    }

    return status;
}

bool FiveMServerTools::isServerRunning(pid_t pid) const {
    if (pid <= 0) return false;
    return kill(pid, 0) == 0;
}

} // namespace fml
