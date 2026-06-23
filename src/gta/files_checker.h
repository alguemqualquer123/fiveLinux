#pragma once

#include <string>
#include <vector>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

struct FileCheckResult {
    std::string path;
    bool exists;
    bool correctSize;
    uint64_t actualSize;
    uint64_t expectedSize;
    bool corrupt;
};

class GtaFilesChecker {
public:
    GtaFilesChecker() = default;

    std::vector<FileCheckResult> checkFiles(const std::string& installDir) const;
    std::vector<std::string> getMissingFiles(const std::string& installDir) const;
    std::vector<std::string> getCorruptFiles(const std::string& installDir) const;
    bool repairFiles(const std::string& installDir);
    float getCompletionPercentage(const std::string& installDir) const;

private:
    std::vector<std::string> getEssentialFiles() const;
    std::vector<std::string> getEssentialDirectories() const;
    uint64_t getExpectedFileSize(const std::string& filename) const;
};

} // namespace fml
