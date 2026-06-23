#pragma once

#include <string>
#include <optional>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

class FiveMLauncher {
public:
    FiveMLauncher() = default;

    bool launch(const std::string& installDir, const std::string& winePath,
                const std::string& serverUrl = "");
    bool isRunning() const;
    bool stop();
    pid_t getProcessId() const;

private:
    bool startProcess(const std::string& winePath, const std::string& exePath,
                      const std::string& prefixPath, const std::string& args);
    void setupEnvironment(const std::string& prefixPath);

    pid_t pid_ = 0;
};

} // namespace fml
