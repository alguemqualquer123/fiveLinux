#include "command_interface.h"

#include "../core/system_detector.h"
#include "../core/environment.h"
#include "../core/config.h"
#include "../core/logger.h"
#include "../wine/prefix_manager.h"
#include "../wine/proton_support.h"
#include "../fivem/installer.h"
#include "../fivem/updater.h"
#include "../fivem/launcher.h"
#include "../fivem/cache_manager.h"
#include "../gta/installation_detector.h"
#include "../gta/files_checker.h"
#include "../gta/repair_system.h"
#include "../graphics/vulkan_checker.h"
#include "../graphics/dxvk_checker.h"
#include "../graphics/gpu_detector.h"
#include "../network/latency_test.h"
#include "../network/firewall_check.h"
#include "../network/port_manager.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

namespace fml {

CommandInterface::CommandInterface() {
    registerBuiltinCommands();
}

std::vector<std::string> CommandInterface::parseArgs(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    return args;
}

void CommandInterface::registerCommand(const CliCommand& cmd) {
    commands_[cmd.name] = cmd;
}

void CommandInterface::printVersion() const {
    std::cout << "fivem-linux " << FML_VERSION << std::endl;
    std::cout << "FiveMLinuxSDK - Advanced FiveM/GTA V support for Linux" << std::endl;
}

void CommandInterface::printHelp() const {
    printVersion();
    std::cout << std::endl << "Usage: fivem-linux <command> [options]" << std::endl << std::endl;
    std::cout << "Commands:" << std::endl;

    size_t maxLen = 0;
    for (auto& [name, cmd] : commands_) {
        maxLen = std::max(maxLen, name.size());
    }

    for (auto& [name, cmd] : commands_) {
        std::cout << "  " << std::left << std::setw(maxLen + 2) << name
                  << cmd.description << std::endl;
    }

    std::cout << std::endl << "Run 'fivem-linux <command> --help' for more info." << std::endl;
}

void CommandInterface::registerBuiltinCommands() {
    commands_["status"] = {
        "status", "Show system status and diagnostics", "status",
        [this](const auto& args) { return cmdStatus(args); }
    };
    commands_["repair"] = {
        "repair", "Auto-repair common issues", "repair [--auto]",
        [this](const auto& args) { return cmdRepair(args); }
    };
    commands_["install"] = {
        "install", "Install FiveM", "install [--wine <path>]",
        [this](const auto& args) { return cmdInstall(args); }
    };
    commands_["diagnose"] = {
        "diagnose", "Run full system diagnostics", "diagnose",
        [this](const auto& args) { return cmdDiagnose(args); }
    };
    commands_["wine"] = {
        "wine", "Wine prefix management", "wine [create|repair|info|delete]",
        [this](const auto& args) { return cmdWine(args); }
    };
    commands_["cache"] = {
        "cache", "Cache management", "cache [clear|size|list]",
        [this](const auto& args) { return cmdCache(args); }
    };
    commands_["launch"] = {
        "launch", "Launch FiveM", "launch [--server <url>]",
        [this](const auto& args) { return cmdLaunch(args); }
    };
    commands_["gpu"] = {
        "gpu", "GPU information", "gpu",
        [this](const auto& args) { return cmdGpu(args); }
    };
    commands_["server"] = {
        "server", "FiveM server management", "server [start|stop|status]",
        [this](const auto& args) { return cmdServer(args); }
    };
    commands_["network"] = {
        "network", "Network diagnostics", "network [ping|ports|firewall]",
        [this](const auto& args) { return cmdNetwork(args); }
    };
}

int CommandInterface::run(int argc, char* argv[]) {
    auto args = parseArgs(argc, argv);

    if (args.empty() || args[0] == "--help" || args[0] == "-h") {
        printHelp();
        return 0;
    }

    if (args[0] == "--version" || args[0] == "-v") {
        printVersion();
        return 0;
    }

    std::string cmd = args[0];
    auto it = commands_.find(cmd);
    if (it == commands_.end()) {
        std::cerr << "Unknown command: " << cmd << std::endl;
        printHelp();
        return 1;
    }

    std::vector<std::string> cmdArgs(args.begin() + 1, args.end());
    return it->second.handler(cmdArgs);
}

int CommandInterface::cmdStatus(const std::vector<std::string>&) {
    SystemDetector detector;
    detector.detect();
    auto sysInfo = detector.getSystemInfo();

    std::cout << "=== System Status ===" << std::endl;
    std::cout << "Distro: " << sysInfo.distro_name << " " << sysInfo.distro_version << std::endl;
    std::cout << "Kernel: " << sysInfo.kernel_version << std::endl;
    std::cout << "Arch: " << sysInfo.arch << std::endl;
    std::cout << "CPU: " << sysInfo.cpu_cores << " cores" << std::endl;
    std::cout << "RAM: " << sysInfo.total_ram_mb << " MB" << std::endl;

    if (detector.isWayland()) std::cout << "Display: Wayland" << std::endl;
    else std::cout << "Display: X11" << std::endl;

    std::cout << std::endl;

    GpuDetector gpu;
    auto gpuInfo = gpu.detect();
    std::cout << "GPU: " << gpuInfo.name << std::endl;
    std::cout << "Driver: " << gpuInfo.driver_version << std::endl;
    std::cout << "Vulkan: " << (gpuInfo.vulkan_supported ? "OK" : "Not available") << std::endl;

    std::cout << std::endl;

    WinePrefixManager prefixMgr;
    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";
    if (prefixPath.empty()) {
        std::cout << "Wine Prefix: HOME not set" << std::endl;
    } else {
        auto info = prefixMgr.getPrefixInfo(prefixPath);
        if (info && info->exists) {
            std::cout << "Wine Prefix: OK (" << (info->is64bit ? "64-bit" : "32-bit") << ")" << std::endl;
        } else {
            std::cout << "Wine Prefix: Not found" << std::endl;
        }
    }

    DxvkChecker dxvk;
    bool dxvkOk = home && std::filesystem::exists(
        std::string(home) + "/.fivem-linux/prefix/drive_c/windows/system32/d3d11.dll");
    std::cout << "DXVK: " << (dxvkOk ? "Installed" : "Not installed") << std::endl;

    FiveMInstaller installer;
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (!fivemPath.empty()) {
        auto info = installer.getInstallInfo(fivemPath);
        std::cout << "FiveM: " << (info.installed ? "Installed (" + info.version + ")" : "Not installed") << std::endl;
    } else {
        std::cout << "FiveM: Not installed" << std::endl;
    }

    return 0;
}

int CommandInterface::cmdRepair(const std::vector<std::string>& args) {
    std::cout << "Running auto-repair..." << std::endl;

    GtaRepairSystem repair;
    const char* home = getenv("HOME");
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (fivemPath.empty()) {
        std::cerr << "FiveM install path not found." << std::endl;
        return 1;
    }

    auto report = repair.diagnose(fivemPath);

    for (auto& check : report.checks) {
        std::string status = check.passed ? "[OK]" : "[FAIL]";
        std::cout << status << " " << check.name << ": " << check.message << std::endl;

        if (!check.passed && !check.fix_suggestion.empty()) {
            std::cout << "  Fix: " << check.fix_suggestion << std::endl;
        }
    }

    if (!report.success) {
        std::cout << std::endl << "Issues found. Run 'fivem-linux diagnose' for details." << std::endl;
    } else {
        std::cout << std::endl << "All checks passed!" << std::endl;
    }

    return report.success ? 0 : 1;
}

int CommandInterface::cmdInstall(const std::vector<std::string>& args) {
    const char* home = getenv("HOME");
    std::string installDir = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    std::string winePath = "wine";

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--wine" && i + 1 < args.size()) {
            winePath = args[i + 1];
            i++;
        } else if (args[i][0] != '-') {
            installDir = args[i];
        }
    }

    if (installDir.empty()) {
        std::cerr << "No install directory specified." << std::endl;
        return 1;
    }

    FiveMInstaller installer;
    if (installer.isInstalled(installDir)) {
        std::cout << "FiveM is already installed at: " << installDir << std::endl;
        return 0;
    }

    auto progress = [](float pct, const std::string& msg) {
        std::cout << "\r[" << std::fixed << std::setprecision(0) << (pct * 100)
                  << "%] " << msg << std::flush;
    };

    bool ok = installer.install(installDir, winePath, progress);
    std::cout << std::endl;
    return ok ? 0 : 1;
}

int CommandInterface::cmdDiagnose(const std::vector<std::string>&) {
    GtaRepairSystem repair;
    const char* home = getenv("HOME");
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";

    if (fivemPath.empty()) {
        std::cerr << "FiveM install path not found." << std::endl;
        return 1;
    }

    auto report = repair.diagnose(fivemPath);

    std::cout << "=== Diagnostic Report ===" << std::endl;
    std::cout << "System: " << report.system.distro_name << " " << report.system.distro_version << std::endl;
    std::cout << "GPU: " << report.gpu.name << std::endl;
    std::cout << "Status: " << (report.success ? "ALL OK" : "ISSUES FOUND") << std::endl;
    std::cout << std::endl;

    for (auto& check : report.checks) {
        std::string icon = check.passed ? "\033[32mOK\033[0m" : "\033[31mFAIL\033[0m";
        std::cout << "[" << icon << "] " << check.name << ": " << check.message << std::endl;

        if (!check.passed && !check.fix_suggestion.empty()) {
            std::cout << "  -> " << check.fix_suggestion << std::endl;
        }
    }

    auto actions = repair.getRecommendedActions(fivemPath);
    if (!actions.empty()) {
        std::cout << std::endl << "Recommended actions: " << actions.size() << std::endl;
    }

    return report.success ? 0 : 1;
}

int CommandInterface::cmdWine(const std::vector<std::string>& args) {
    if (args.empty() || args[0] == "--help") {
        std::cout << "Wine prefix management:" << std::endl;
        std::cout << "  create   Create a new Wine prefix" << std::endl;
        std::cout << "  repair   Repair existing prefix" << std::endl;
        std::cout << "  info     Show prefix information" << std::endl;
        std::cout << "  delete   Delete prefix" << std::endl;
        std::cout << "  proton   Show Proton versions" << std::endl;
        return 0;
    }

    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";
    WinePrefixManager mgr;

    if (args[0] == "create") {
        std::cout << "Creating Wine prefix at: " << prefixPath << std::endl;
        auto result = mgr.createPrefix(prefixPath, "wine", true);
        if (result) {
            std::cout << "Prefix created successfully." << std::endl;
            return 0;
        }
        std::cerr << "Failed to create prefix." << std::endl;
        return 1;
    } else if (args[0] == "repair") {
        std::cout << "Repairing prefix..." << std::endl;
        return mgr.repairPrefix(prefixPath) ? 0 : 1;
    } else if (args[0] == "info") {
        auto info = mgr.getPrefixInfo(prefixPath);
        if (info && info->exists) {
            std::cout << "Path: " << info->path << std::endl;
            std::cout << "64-bit: " << (info->is64bit ? "Yes" : "No") << std::endl;
            std::cout << "Size: " << (info->sizeBytes / 1024 / 1024) << " MB" << std::endl;
        } else {
            std::cout << "No prefix found." << std::endl;
        }
        return 0;
    } else if (args[0] == "delete") {
        return mgr.deletePrefix(prefixPath) ? 0 : 1;
    } else if (args[0] == "proton") {
        ProtonSupport proton;
        proton.detectProton();
        auto versions = proton.getProtonVersions();
        if (versions.empty()) {
            std::cout << "No Proton versions found." << std::endl;
        } else {
            std::cout << "Proton versions:" << std::endl;
            for (auto& v : versions) {
                std::cout << "  " << v.name << " (" << v.version << ")" << std::endl;
            }
        }
        return 0;
    }

    std::cerr << "Unknown wine command: " << args[0] << std::endl;
    return 1;
}

int CommandInterface::cmdCache(const std::vector<std::string>& args) {
    const char* home = getenv("HOME");
    std::string cacheDir = home ? std::string(home) + "/.fivem-linux/FiveM/cache" : "";

    if (cacheDir.empty() || !std::filesystem::exists(cacheDir)) {
        std::cout << "Cache directory not found." << std::endl;
        return 1;
    }

    FiveMCacheManager cache;

    if (args.empty() || args[0] == "size") {
        auto stats = cache.getCacheSize(cacheDir);
        std::cout << "Cache size: " << (stats.totalSizeBytes / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Game: " << (stats.gameCacheSize / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Priv: " << (stats.privCacheSize / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Files: " << (stats.filesCacheSize / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Server: " << (stats.serverCacheSize / 1024 / 1024) << " MB" << std::endl;
        std::cout << "  Entries: " << stats.entryCount << std::endl;
        return 0;
    } else if (args[0] == "clear") {
        std::string type = (args.size() > 1) ? args[1] : "all";
        CacheType ct = CacheType::All;
        if (type == "game") ct = CacheType::Game;
        else if (type == "priv") ct = CacheType::Priv;
        else if (type == "files") ct = CacheType::Files;

        return cache.clearCache(cacheDir, ct) ? 0 : 1;
    } else if (args[0] == "list") {
        auto entries = cache.listCacheDirs(cacheDir);
        for (auto& e : entries) {
            std::cout << (e.isDirectory ? "d " : "f ")
                      << std::setw(10) << (e.sizeBytes / 1024) << " KB  "
                      << e.name << std::endl;
        }
        return 0;
    }

    std::cerr << "Unknown cache command: " << args[0] << std::endl;
    return 1;
}

int CommandInterface::cmdLaunch(const std::vector<std::string>& args) {
    const char* home = getenv("HOME");
    std::string installDir = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    std::string winePath = "wine";
    std::string serverUrl;

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--server" && i + 1 < args.size()) {
            serverUrl = args[i + 1];
            i++;
        } else if (args[i] == "--wine" && i + 1 < args.size()) {
            winePath = args[i + 1];
            i++;
        }
    }

    FiveMLauncher launcher;
    if (launcher.launch(installDir, winePath, serverUrl)) {
        std::cout << "FiveM launched. PID: " << launcher.getProcessId() << std::endl;
        return 0;
    }

    std::cerr << "Failed to launch FiveM." << std::endl;
    return 1;
}

int CommandInterface::cmdGpu(const std::vector<std::string>&) {
    GpuDetector gpu;
    auto info = gpu.detect();

    std::cout << "=== GPU Information ===" << std::endl;
    std::cout << "Name: " << info.name << std::endl;

    std::string vendorStr = "Unknown";
    switch (info.vendor) {
        case GpuVendor::NVIDIA: vendorStr = "NVIDIA"; break;
        case GpuVendor::AMD: vendorStr = "AMD"; break;
        case GpuVendor::Intel: vendorStr = "Intel"; break;
        default: break;
    }
    std::cout << "Vendor: " << vendorStr << std::endl;
    std::cout << "Driver: " << info.driver_version << std::endl;
    std::cout << "VRAM: " << (info.vram_mb > 0 ? std::to_string(info.vram_mb) + " MB" : "Unknown") << std::endl;
    std::cout << "Vulkan: " << (info.vulkan_supported ? "Supported" : "Not supported") << std::endl;

    if (!info.vulkan_version.empty()) {
        std::cout << "Vulkan API: " << info.vulkan_version << std::endl;
    }

    return 0;
}

int CommandInterface::cmdServer(const std::vector<std::string>& args) {
    if (args.empty() || args[0] == "--help") {
        std::cout << "FiveM server management:" << std::endl;
        std::cout << "  install  Install FiveM server" << std::endl;
        std::cout << "  start    Start server" << std::endl;
        std::cout << "  stop     Stop server" << std::endl;
        std::cout << "  status   Server status" << std::endl;
        return 0;
    }

    std::cerr << "Server command not yet fully implemented." << std::endl;
    return 1;
}

int CommandInterface::cmdNetwork(const std::vector<std::string>& args) {
    if (args.empty() || args[0] == "ping") {
        LatencyTest test;
        std::cout << "Testing FiveM server connectivity..." << std::endl;
        auto results = test.testFiveMServers();
        for (auto& r : results) {
            std::string status = r.reachable ? "OK" : "UNREACHABLE";
            std::cout << "  " << r.host << ": " << status;
            if (r.reachable) std::cout << " (" << std::fixed << std::setprecision(1) << r.latencyMs << "ms)";
            std::cout << std::endl;
        }
        return 0;
    } else if (args[0] == "ports") {
        PortManager portMgr;
        auto ports = portMgr.checkRequiredPorts();
        std::cout << "Required ports:" << std::endl;
        for (auto& p : ports) {
            std::cout << "  " << p.port << "/" << p.protocol
                      << ": " << (p.open ? "OPEN" : "CLOSED") << std::endl;
        }
        return 0;
    } else if (args[0] == "firewall") {
        FirewallCheck fw;
        std::cout << "Firewall: " << fw.getFirewallStatus() << std::endl;
        return 0;
    }

    std::cerr << "Unknown network command: " << args[0] << std::endl;
    return 1;
}

} // namespace fml
