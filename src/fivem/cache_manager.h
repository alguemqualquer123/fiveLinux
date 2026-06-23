#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

enum class CacheType {
    Game,
    Priv,
    Files,
    All,
    ServerPriv
};

struct CacheEntry {
    std::string path;
    std::string name;
    uint64_t sizeBytes;
    bool isDirectory;
};

struct CacheStats {
    uint64_t totalSizeBytes;
    uint64_t gameCacheSize;
    uint64_t privCacheSize;
    uint64_t filesCacheSize;
    uint64_t serverCacheSize;
    uint32_t entryCount;
};

class FiveMCacheManager {
public:
    FiveMCacheManager() = default;

    bool clearCache(const std::string& cacheDir, CacheType type);
    CacheStats getCacheSize(const std::string& cacheDir) const;
    std::vector<CacheEntry> listCacheDirs(const std::string& cacheDir) const;
    bool optimizeCache(const std::string& cacheDir);

private:
    uint64_t calculateDirSize(const std::string& path) const;
    bool removeDirContents(const std::string& path);
};

} // namespace fml
