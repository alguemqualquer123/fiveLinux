#pragma once

#include <string>
#include <optional>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

struct ServerStatus {
    bool running;
    pid_t pid;
    uint32_t port;
    uint32_t uptimeSeconds;
    uint32_t playerCount;
    std::string version;
};

class FiveMServerTools {
public:
    FiveMServerTools() = default;

    bool installServer(const std::string& installDir, ProgressCallback progress = nullptr);
    bool updateServer(const std::string& installDir, ProgressCallback progress = nullptr);
    pid_t startServer(const std::string& serverDir, uint32_t port = 30120);
    bool stopServer(pid_t pid);
    ServerStatus getServerStatus(pid_t pid) const;
    bool isServerRunning(pid_t pid) const;

private:
    bool downloadServerArtifacts(const std::string& targetDir, ProgressCallback progress = nullptr);
    bool extractServer(const std::string& archivePath, const std::string& targetDir);
};

} // namespace fml
