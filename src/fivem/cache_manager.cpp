#include "cache_manager.h"

#include <algorithm>

namespace fml {

uint64_t FiveMCacheManager::calculateDirSize(const std::string& path) const {
    uint64_t totalSize = 0;
    if (!std::filesystem::exists(path)) return 0;

    for (auto& entry : std::filesystem::recursive_directory_iterator(path,
            std::filesystem::directory_options::skip_permission_denied)) {
        if (entry.is_regular_file()) {
            std::error_code ec;
            totalSize += entry.file_size(ec);
        }
    }
    return totalSize;
}

bool FiveMCacheManager::removeDirContents(const std::string& path) {
    if (!std::filesystem::exists(path)) return true;

    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        std::filesystem::remove_all(entry.path(), ec);
    }
    return !ec;
}

bool FiveMCacheManager::clearCache(const std::string& cacheDir, CacheType type) {
    if (!std::filesystem::exists(cacheDir)) return false;

    auto clear = [this, &cacheDir](const std::string& subdir) {
        auto path = std::filesystem::path(cacheDir) / subdir;
        if (std::filesystem::exists(path)) {
            return removeDirContents(path.string());
        }
        return true;
    };

    bool result = true;

    switch (type) {
        case CacheType::Game:
            result = clear("game");
            break;
        case CacheType::Priv:
            result = clear("priv");
            break;
        case CacheType::Files:
            result = clear("files");
            break;
        case CacheType::All:
            result = clear("game") && clear("priv") && clear("files");
            break;
        case CacheType::ServerPriv:
            result = clear("server-cache-priv");
            break;
    }

    return result;
}

CacheStats FiveMCacheManager::getCacheSize(const std::string& cacheDir) const {
    CacheStats stats{};
    if (!std::filesystem::exists(cacheDir)) return stats;

    stats.gameCacheSize = calculateDirSize(cacheDir + "/game");
    stats.privCacheSize = calculateDirSize(cacheDir + "/priv");
    stats.filesCacheSize = calculateDirSize(cacheDir + "/files");
    stats.serverCacheSize = calculateDirSize(cacheDir + "/server-cache-priv");
    stats.totalSizeBytes = stats.gameCacheSize + stats.privCacheSize +
                           stats.filesCacheSize + stats.serverCacheSize;

    for (auto& entry : std::filesystem::recursive_directory_iterator(cacheDir,
            std::filesystem::directory_options::skip_permission_denied)) {
        if (entry.is_regular_file()) {
            stats.entryCount++;
        }
    }

    return stats;
}

std::vector<CacheEntry> FiveMCacheManager::listCacheDirs(const std::string& cacheDir) const {
    std::vector<CacheEntry> entries;
    if (!std::filesystem::exists(cacheDir)) return entries;

    for (auto& entry : std::filesystem::directory_iterator(cacheDir)) {
        CacheEntry cacheEntry{};
        cacheEntry.name = entry.path().filename().string();
        cacheEntry.path = entry.path().string();
        cacheEntry.isDirectory = entry.is_directory();

        if (cacheEntry.isDirectory) {
            cacheEntry.sizeBytes = calculateDirSize(cacheEntry.path);
        } else {
            cacheEntry.sizeBytes = entry.file_size();
        }

        entries.push_back(cacheEntry);
    }

    return entries;
}

bool FiveMCacheManager::optimizeCache(const std::string& cacheDir) {
    if (!std::filesystem::exists(cacheDir)) return false;

    auto gameCache = std::filesystem::path(cacheDir) / "game";
    if (std::filesystem::exists(gameCache)) {
        for (auto& entry : std::filesystem::directory_iterator(gameCache)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                if (ext == ".tmp" || ext == ".bak" || ext == ".old") {
                    std::error_code ec;
                    std::filesystem::remove(entry.path(), ec);
                }
            }
        }
    }

    return true;
}

} // namespace fml
