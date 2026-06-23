#include <cassert>
#include <iostream>
#include <string>

#include "core/system_detector.h"
#include "core/environment.h"
#include "core/config.h"
#include "core/logger.h"

#define TEST(name) static bool test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    if (test_##name()) { std::cout << "PASS" << std::endl; passed++; } \
    else { std::cout << "FAIL" << std::endl; failed++; } \
} while(0)

TEST(system_detector_detect) {
    fml::SystemDetector detector;
    bool result = detector.detect();
    auto info = detector.getSystemInfo();
    return result && !info.kernel_version.empty();
}

TEST(system_detector_uptime) {
    fml::SystemDetector detector;
    detector.detect();
    auto uptime = detector.getUptime();
    return uptime > 0;
}

TEST(environment_home) {
    std::string home = fml::Environment::homeDir();
    return !home.empty();
}

TEST(environment_expand) {
    std::string expanded = fml::Environment::expandPath("~/test");
    return expanded.find("~") == std::string::npos;
}

TEST(logger_init) {
    auto& logger = fml::Logger::instance();
    logger.init("/tmp/fml_test_logs");
    logger.log(fml::LogLevel::Info, "test", "test message");
    logger.flush();
    return std::filesystem::exists("/tmp/fml_test_logs");
}

int main() {
    int passed = 0, failed = 0;

    std::cout << "=== FiveMLinuxSDK Tests ===" << std::endl;

    RUN_TEST(system_detector_detect);
    RUN_TEST(system_detector_uptime);
    RUN_TEST(environment_home);
    RUN_TEST(environment_expand);
    RUN_TEST(logger_init);

    std::cout << std::endl << "Results: " << passed << " passed, " << failed << " failed" << std::endl;

    std::filesystem::remove_all("/tmp/fml_test_logs");

    return failed > 0 ? 1 : 0;
}
