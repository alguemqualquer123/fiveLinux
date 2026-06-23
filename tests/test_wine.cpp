#include <cassert>
#include <iostream>
#include <string>

#include "wine/prefix_manager.h"
#include "wine/dll_manager.h"
#include "wine/proton_support.h"

#define TEST(name) static bool test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    if (test_##name()) { std::cout << "PASS" << std::endl; passed++; } \
    else { std::cout << "FAIL" << std::endl; failed++; } \
} while(0)

TEST(prefix_not_exists) {
    fml::WinePrefixManager mgr;
    return !mgr.prefixExists("/nonexistent/path/prefix");
}

TEST(prefix_get_info) {
    fml::WinePrefixManager mgr;
    auto info = mgr.getPrefixInfo("/nonexistent/path/prefix");
    return info.has_value() && !info->exists;
}

TEST(proton_detect) {
    fml::ProtonSupport proton;
    proton.detectProton();
    auto versions = proton.getProtonVersions();
    return true;
}

TEST(dll_manager_overrides) {
    fml::DllManager mgr("/tmp/test_prefix");
    auto overrides = mgr.getDllOverrides();
    return true;
}

int main() {
    int passed = 0, failed = 0;

    std::cout << "=== Wine Module Tests ===" << std::endl;

    RUN_TEST(prefix_not_exists);
    RUN_TEST(prefix_get_info);
    RUN_TEST(proton_detect);
    RUN_TEST(dll_manager_overrides);

    std::cout << std::endl << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
    return failed > 0 ? 1 : 0;
}
