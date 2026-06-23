#pragma once

#include <string>
#include <optional>
#include <filesystem>

#include "fivemlinux/types.h"

namespace fml {

struct PrefixInfo {
    std::string path;
    bool exists;
    bool is64bit;
    std::string wineVersion;
    uint64_t sizeBytes;
};

class WinePrefixManager {
public:
    WinePrefixManager() = default;

    std::optional<PrefixInfo> createPrefix(const std::string& path,
                                           const std::string& winePath,
                                           bool is64bit = true);
    bool deletePrefix(const std::string& path);
    bool backupPrefix(const std::string& path, const std::string& backupPath);
    bool restorePrefix(const std::string& backupPath, const std::string& targetPath);
    bool repairPrefix(const std::string& path);
    std::optional<PrefixInfo> getPrefixInfo(const std::string& path);
    bool prefixExists(const std::string& path) const;

private:
    bool runWineCommand(const std::string& winePath,
                        const std::string& prefixPath,
                        const std::string& args);
    bool createDirectoryStructure(const std::string& path, bool is64bit);
    bool initializeRegistry(const std::string& prefixPath, bool is64bit);
    bool runWineboot(const std::string& winePath, const std::string& prefixPath);
};

} // namespace fml
