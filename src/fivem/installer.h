#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "fivemlinux/types.h"

namespace fml {

struct FiveMInstallInfo {
    bool installed;
    std::string installPath;
    std::string version;
    uint64_t totalSizeBytes;
    bool hasExe;
    bool hasCitizenFX;
    bool hasCache;
    FiveMStatus status;
};

class FiveMInstaller {
public:
    FiveMInstaller() = default;

    bool install(const std::string& installDir, const std::string& winePath,
                 ProgressCallback progress = nullptr);
    bool uninstall(const std::string& installDir);
    bool isInstalled(const std::string& installDir) const;
    FiveMInstallInfo getInstallInfo(const std::string& installDir) const;

private:
    bool downloadFiveM(const std::string& targetDir, ProgressCallback progress);
    bool extractArchive(const std::string& archivePath, const std::string& targetDir);
    bool createCacheStructure(const std::string& installDir);
    bool downloadFile(const std::string& url, const std::string& outputPath,
                      ProgressCallback progress = nullptr);
    std::string fetchBuildInfo();
    std::string extractUrlFromJson(const std::string& json);
};

} // namespace fml
