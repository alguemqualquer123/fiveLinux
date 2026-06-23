#include "fivemlinux/fivemlinux.h"
#include "core/system_detector.h"
#include "core/environment.h"
#include "graphics/gpu_detector.h"
#include "wine/proton_support.h"
#include "fivem/installer.h"
#include "gta/repair_system.h"
#include "network/firewall_check.h"

#include <iostream>

namespace fml {

FiveMLinuxSDK& FiveMLinuxSDK::instance() {
    static FiveMLinuxSDK sdk;
    return sdk;
}

bool FiveMLinuxSDK::initialize() {
    if (initialized_) return true;

    Logger::instance().init(Environment::logDir());
    Logger::instance().log(LogLevel::Info, "SDK", "FiveMLinuxSDK " + std::string(FML_VERSION) + " initializing");

    initialized_ = true;
    return true;
}

void FiveMLinuxSDK::shutdown() {
    if (!initialized_) return;
    Logger::instance().log(LogLevel::Info, "SDK", "FiveMLinuxSDK shutting down");
    Logger::instance().flush();
    initialized_ = false;
}

std::string FiveMLinuxSDK::getVersion() const {
    return FML_VERSION;
}

SystemInfo FiveMLinuxSDK::getSystemInfo() const {
    SystemDetector detector;
    detector.detect();
    return detector.getSystemInfo();
}

GpuInfo FiveMLinuxSDK::getGpuInfo() const {
    GpuDetector gpu;
    return gpu.detect();
}

WineInfo FiveMLinuxSDK::getWineInfo() const {
    ProtonSupport proton;
    proton.detectProton();
    auto recommended = proton.getRecommendedProton();
    if (recommended) {
        auto info = proton.getWineInfoFromProton(*recommended);
        if (info) return *info;
    }
    return {};
}

GraphicsState FiveMLinuxSDK::getGraphicsState() const {
    GraphicsState state{};
    GpuDetector gpu;
    auto gpuInfo = gpu.detect();
    state.gpu_vendor = gpuInfo.vendor;
    state.gpu_name = gpuInfo.name;
    state.vulkan_available = gpuInfo.vulkan_supported;
    state.driver_version = gpuInfo.driver_version;
    return state;
}

FiveMState FiveMLinuxSDK::getFiveMState() const {
    const char* home = getenv("HOME");
    std::string installDir = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (installDir.empty()) return {};

    FiveMInstaller installer;
    auto info = installer.getInstallInfo(installDir);

    FiveMState state{};
    state.status = info.status;
    state.version = info.version;
    state.install_path = info.installPath;
    state.has_gta_files = info.hasExe;
    return state;
}

NetworkInfo FiveMLinuxSDK::getNetworkInfo() const {
    NetworkInfo info{};
    FirewallCheck fw;
    info.firewall_active = fw.isFirewallActive();
    return info;
}

bool FiveMLinuxSDK::repair() {
    const char* home = getenv("HOME");
    std::string installDir = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (installDir.empty()) return false;

    GtaRepairSystem repair;
    auto report = repair.diagnose(installDir);
    for (auto& action : repair.getRecommendedActions(installDir)) {
        repair.repair(installDir, action);
    }
    return true;
}

bool FiveMLinuxSDK::diagnose() {
    const char* home = getenv("HOME");
    std::string installDir = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (installDir.empty()) return false;

    GtaRepairSystem repair;
    auto report = repair.diagnose(installDir);
    return report.success;
}

bool FiveMLinuxSDK::createWinePrefix(const std::string& path) {
    WinePrefixManager mgr;
    auto result = mgr.createPrefix(path, "wine", true);
    return result.has_value();
}

bool FiveMLinuxSDK::installFiveM(const std::string& installDir) {
    FiveMInstaller installer;
    return installer.install(installDir, "wine");
}

bool FiveMLinuxSDK::launchFiveM(const std::string& installDir) {
    FiveMLauncher launcher;
    return launcher.launch(installDir, "wine");
}

} // namespace fml
