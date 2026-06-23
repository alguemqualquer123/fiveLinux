#pragma once

#include <string>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

class Vkd3dChecker {
public:
    Vkd3dChecker() = default;

    bool isInstalled(const std::string& prefixPath) const;
    std::string getVersion(const std::string& prefixPath) const;
    bool install(const std::string& prefixPath, const std::string& version = "latest");
    bool uninstall(const std::string& prefixPath);
    std::string getLatestVersion() const;

private:
    std::string getDllVersion(const std::string& dllPath) const;
    bool downloadVkd3d(const std::string& version, const std::string& targetDir);
};

} // namespace fml
