#include <cassert>
#include <iostream>
#include <string>

#include "fivem/installer.h"
#include "fivem/updater.h"
#include "fivem/cache_manager.h"

#define TEST(name) static bool test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    if (test_##name()) { std::cout << "PASS" << std::endl; passed++; } \
    else { std::cout << "FAIL" << std::endl; failed++; } \
} while(0)

TEST(installer_not_installed) {
    fml::FiveMInstaller installer;
    return !installer.isInstalled("/nonexistent/path");
}

TEST(installer_get_info) {
    fml::FiveMInstaller installer;
    auto info = installer.getInstallInfo("/nonexistent/path");
    return info.status == fml::FiveMStatus::NotInstalled;
}

TEST(cache_manager_empty) {
    fml::FiveMCacheManager cache;
    auto stats = cache.getCacheSize("/nonexistent/path");
    return stats.totalSizeBytes == 0;
}

int main() {
    int passed = 0, failed = 0;

    std::cout << "=== FiveM Module Tests ===" << std::endl;

    RUN_TEST(installer_not_installed);
    RUN_TEST(installer_get_info);
    RUN_TEST(cache_manager_empty);

    std::cout << std::endl << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
    return failed > 0 ? 1 : 0;
}
