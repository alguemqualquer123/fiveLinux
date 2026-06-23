#include <cassert>
#include <iostream>
#include <string>

#include "graphics/vulkan_checker.h"
#include "graphics/dxvk_checker.h"
#include "graphics/gpu_detector.h"

#define TEST(name) static bool test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    if (test_##name()) { std::cout << "PASS" << std::endl; passed++; } \
    else { std::cout << "FAIL" << std::endl; failed++; } \
} while(0)

TEST(gpu_detect) {
    fml::GpuDetector gpu;
    auto info = gpu.detect();
    return true;
}

TEST(vulkan_check) {
    fml::VulkanChecker vk;
    bool available = vk.isAvailable();
    return true;
}

TEST(dxvk_not_installed) {
    fml::DxvkChecker dxvk;
    return !dxvk.isInstalled("/nonexistent/prefix");
}

int main() {
    int passed = 0, failed = 0;

    std::cout << "=== Graphics Module Tests ===" << std::endl;

    RUN_TEST(gpu_detect);
    RUN_TEST(vulkan_check);
    RUN_TEST(dxvk_not_installed);

    std::cout << std::endl << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
    return failed > 0 ? 1 : 0;
}
