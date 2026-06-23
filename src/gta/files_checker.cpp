#include "files_checker.h"

#include <algorithm>
#include <fstream>

namespace fml {

std::vector<std::string> GtaFilesChecker::getEssentialFiles() const {
    return {
        "GTA5.exe",
        "PlayGTAV.exe",
        "GTAVLauncher.exe",
        "x64a.rpf",
        "x64b.rpf",
        "x64c.rpf",
        "x64d.rpf",
        "x64e.rpf",
        "x64f.rpf",
        "x64g.rpf",
        "x64h.rpf",
        "x64i.rpf",
        "x64j.rpf",
        "x64k.rpf",
        "x64l.rpf",
        "update.rpf",
        "bink2w64.dll",
        "d3dcompiler_46.dll",
        "GFSDK_SSAO_Win64.dll",
        "GFSDK_SSAO_D3D11_Win64.dll",
        "libcef.dll"
    };
}

std::vector<std::string> GtaFilesChecker::getEssentialDirectories() const {
    return {
        "update",
        "update/update.rpf",
        "dlcpacks",
        "common",
        "common/data",
        "x64",
        "x64/audio"
    };
}

uint64_t GtaFilesChecker::getExpectedFileSize(const std::string& /*filename*/) const {
    return 0;
}

std::vector<FileCheckResult> GtaFilesChecker::checkFiles(const std::string& installDir) const {
    std::vector<FileCheckResult> results;

    for (auto& file : getEssentialFiles()) {
        FileCheckResult result{};
        auto fullPath = std::filesystem::path(installDir) / file;
        result.path = file;
        result.exists = std::filesystem::exists(fullPath);

        if (result.exists) {
            std::error_code ec;
            result.actualSize = std::filesystem::file_size(fullPath, ec);
            result.correctSize = !ec;
            result.corrupt = ec;
        } else {
            result.actualSize = 0;
            result.correctSize = false;
            result.corrupt = false;
        }

        results.push_back(result);
    }

    for (auto& dir : getEssentialDirectories()) {
        FileCheckResult result{};
        auto fullPath = std::filesystem::path(installDir) / dir;
        result.path = dir;
        result.exists = std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath);
        result.actualSize = 0;
        result.correctSize = result.exists;
        result.corrupt = false;
        results.push_back(result);
    }

    return results;
}

std::vector<std::string> GtaFilesChecker::getMissingFiles(const std::string& installDir) const {
    std::vector<std::string> missing;
    auto results = checkFiles(installDir);
    for (auto& r : results) {
        if (!r.exists) missing.push_back(r.path);
    }
    return missing;
}

std::vector<std::string> GtaFilesChecker::getCorruptFiles(const std::string& installDir) const {
    std::vector<std::string> corrupt;
    auto results = checkFiles(installDir);
    for (auto& r : results) {
        if (r.exists && r.corrupt) corrupt.push_back(r.path);
    }
    return corrupt;
}

bool GtaFilesChecker::repairFiles(const std::string& /*installDir*/) {
    return false;
}

float GtaFilesChecker::getCompletionPercentage(const std::string& installDir) const {
    auto results = checkFiles(installDir);
    if (results.empty()) return 0.0f;

    uint32_t valid = 0;
    for (auto& r : results) {
        if (r.exists && r.correctSize) valid++;
    }

    return static_cast<float>(valid) / static_cast<float>(results.size()) * 100.0f;
}

} // namespace fml
