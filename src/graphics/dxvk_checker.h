#pragma once

#include <string>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

class DxvkChecker {
public:
    DxvkChecker() = default;

    bool isInstalled(const std::string& prefixPath) const;
    std::string getVersion(const std::string& prefixPath) const;
    bool install(const std::string& prefixPath, const std::string& version = "latest");
    bool uninstall(const std::string& prefixPath);
    std::string getLatestVersion() const;
    bool isSupported() const;

private:
    bool copyDlls(const std::string& dxvkDir, const std::string& system32,
                  const std::string& syswow64, bool is64bit);
    std::string getDllVersion(const std::string& dllPath) const;
    bool downloadDxvk(const std::string& version, const std::string& targetDir);
};

} // namespace fml
