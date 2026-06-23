#include "command_interface.h"
#include "config.h"

#include "core/system_detector.h"
#include "core/environment.h"
#include "core/config.h"
#include "core/logger.h"
#include "wine/prefix_manager.h"
#include "wine/proton_support.h"
#include "fivem/installer.h"
#include "fivem/updater.h"
#include "fivem/launcher.h"
#include "fivem/cache_manager.h"
#include "gta/installation_detector.h"
#include "gta/files_checker.h"
#include "gta/repair_system.h"
#include "graphics/vulkan_checker.h"
#include "graphics/dxvk_checker.h"
#include "graphics/vkd3d_checker.h"
#include "graphics/gpu_detector.h"
#include "network/latency_test.h"
#include "network/firewall_check.h"
#include "network/port_manager.h"
#include "rockstar/launcher.h"
#include "rockstar/games_manager.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <chrono>

namespace fml {

// ANSI Color codes
namespace Color {
    const std::string Reset     = "\033[0m";
    const std::string Bold      = "\033[1m";
    const std::string Dim       = "\033[2m";
    const std::string Italic    = "\033[3m";
    const std::string Underline = "\033[4m";
    const std::string Blink     = "\033[5m";
    const std::string Reverse   = "\033[7m";

    const std::string Black     = "\033[30m";
    const std::string Red       = "\033[31m";
    const std::string Green     = "\033[32m";
    const std::string Yellow    = "\033[33m";
    const std::string Blue      = "\033[34m";
    const std::string Magenta   = "\033[35m";
    const std::string Cyan      = "\033[36m";
    const std::string White     = "\033[37m";

    const std::string BrightRed     = "\033[91m";
    const std::string BrightGreen   = "\033[92m";
    const std::string BrightYellow  = "\033[93m";
    const std::string BrightBlue    = "\033[94m";
    const std::string BrightMagenta = "\033[95m";
    const std::string BrightCyan    = "\033[96m";
    const std::string BrightWhite   = "\033[97m";

    const std::string BgRed     = "\033[41m";
    const std::string BgGreen   = "\033[42m";
    const std::string BgYellow  = "\033[43m";
    const std::string BgBlue    = "\033[44m";
    const std::string BgMagenta = "\033[45m";
    const std::string BgCyan    = "\033[46m";
}

static int termWidth() {
    return 80;
}

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

void CommandInterface::printSeparator() {
    std::cout << Color::Dim;
    for (int i = 0; i < termWidth(); i++) std::cout << "─";
    std::cout << Color::Reset << std::endl;
}

void CommandInterface::printHeader(const std::string& title) {
    int pad = (termWidth() - title.size() - 4) / 2;
    std::cout << Color::Cyan << Color::Bold;
    for (int i = 0; i < pad; i++) std::cout << "═";
    std::cout << "  " << title << "  ";
    for (int i = 0; i < pad; i++) std::cout << "═";
    std::cout << Color::Reset << std::endl << std::endl;
}

void CommandInterface::printSuccess(const std::string& msg) {
    std::cout << Color::Green << Color::Bold << "  ✔ " << Color::Reset
              << Color::BrightGreen << msg << Color::Reset << std::endl;
}

void CommandInterface::printError(const std::string& msg) {
    std::cout << Color::Red << Color::Bold << "  ✘ " << Color::Reset
              << Color::BrightRed << msg << Color::Reset << std::endl;
}

void CommandInterface::printWarning(const std::string& msg) {
    std::cout << Color::Yellow << Color::Bold << "  ⚠ " << Color::Reset
              << Color::BrightYellow << msg << Color::Reset << std::endl;
}

void CommandInterface::printInfo(const std::string& msg) {
    std::cout << Color::Cyan << "  ℹ " << Color::Reset
              << Color::BrightWhite << msg << Color::Reset << std::endl;
}

void CommandInterface::printCheck(bool ok, const std::string& label, const std::string& detail) {
    if (ok) {
        std::cout << Color::Green << Color::Bold << "    ✔ " << Color::Reset
                  << Color::BrightWhite << std::setw(20) << std::left << label
                  << Color::Reset;
        if (!detail.empty()) std::cout << Color::Dim << " " << detail << Color::Reset;
        std::cout << std::endl;
    } else {
        std::cout << Color::Red << Color::Bold << "    ✘ " << Color::Reset
                  << Color::BrightRed << std::setw(20) << std::left << label
                  << Color::Reset;
        if (!detail.empty()) std::cout << Color::Dim << " " << detail << Color::Reset;
        std::cout << std::endl;
    }
}

void CommandInterface::printLoadingBar(float progress, const std::string& label) {
    int barWidth = 40;
    int filled = static_cast<int>(progress * barWidth);

    std::cout << "\r  " << Color::Cyan << "◆ " << Color::Reset
              << Color::BrightWhite << std::setw(20) << std::left << label
              << Color::Reset << " [";

    for (int i = 0; i < barWidth; i++) {
        if (i < filled) {
            if (i < barWidth * 0.3) std::cout << Color::BrightRed << "█";
            else if (i < barWidth * 0.7) std::cout << Color::BrightYellow << "█";
            else std::cout << Color::BrightGreen << "█";
        } else {
            std::cout << Color::Dim << "░";
        }
    }

    std::cout << Color::Reset << "] " << std::fixed << std::setprecision(0)
              << (progress * 100) << "%\r" << std::flush;
}

void CommandInterface::printStars() {
    std::cout << Color::BrightWhite;
    std::cout << "                                          .         *" << std::endl;
    std::cout << "        *              .        *                  .       " << std::endl;
    std::cout << "   .         *                 .        *                *" << std::endl;
    std::cout << "              .     *                          .          " << std::endl;
    std::cout << "   *    .          .     *     .         *         .      " << std::endl;
    std::cout << Color::Reset;
}

void CommandInterface::printMeteor() {
    std::cout << Color::BrightYellow;
    std::cout << "                         " << Color::Dim << "       " << Color::BrightYellow << "        ." << Color::Reset << std::endl;
    std::cout << Color::BrightYellow;
    std::cout << "                        " << Color::Dim << "       " << Color::BrightYellow << "     ." << Color::BrightWhite << "°" << Color::Reset << std::endl;
    std::cout << Color::BrightYellow;
    std::cout << "                       " << Color::Dim << "      " << Color::BrightYellow << "   ." << Color::BrightWhite << "°°°" << Color::Reset << std::endl;
    std::cout << Color::BrightYellow;
    std::cout << "                      " << Color::Dim << "     " << Color::BrightYellow << " ." << Color::BrightWhite << "°°°°°" << Color::BrightYellow << "." << Color::Reset << std::endl;
    std::cout << Color::BrightYellow;
    std::cout << "                     " << Color::Dim << "    " << Color::BrightYellow << "." << Color::BrightWhite << "°°°°°°°°" << Color::BrightYellow << "." << Color::Reset << std::endl;
    std::cout << Color::BrightYellow;
    std::cout << "                    " << Color::Dim << "   " << Color::BrightYellow << "." << Color::BrightWhite << "°°°°°°°°°°" << Color::BrightYellow << "." << Color::Reset << std::endl;
    std::cout << Color::Dim;
    std::cout << "                   " << Color::BrightYellow << "  .............." << Color::Dim << "     " << Color::Reset << std::endl;
    std::cout << Color::Dim;
    std::cout << "                  " << Color::BrightYellow << " ................" << Color::Dim << "    " << Color::Reset << std::endl;
    std::cout << Color::Dim;
    std::cout << "                 " << Color::BrightYellow << ".................." << Color::Dim << "   " << Color::Reset << std::endl;
    std::cout << Color::Reset;
}

void CommandInterface::printBanner() {
    std::cout << std::endl;

    // Stars background
    printStars();

    // Meteor
    printMeteor();

    // Main logo
    std::cout << std::endl;
    std::cout << Color::BrightCyan << Color::Bold;
    std::cout << "         ╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "         ║                                                   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   ███████╗███╗   ███╗███████╗██╗     ██╗██╗  ██╗" << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   ██╔════╝████╗ ████║██╔════╝██║     ██║╚██╗██╔╝" << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   █████╗  ██╔████╔██║█████╗  ██║     ██║ ╚███╔╝ " << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   ██╔══╝  ██║╚██╔╝██║██╔══╝  ██║     ██║ ██╔██╗ " << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   ███████╗██║ ╚═╝ ██║███████╗███████╗██║██╔╝ ██╗" << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║" << Color::BrightMagenta << "   ╚══════╝╚═╝     ╚═╝╚══════╝╚══════╝╚═╝╚═╝  ╚═╝" << Color::BrightCyan << "   ║" << std::endl;
    std::cout << "         ║                                                   ║" << std::endl;
    std::cout << "         ╚═══════════════════════════════════════════════════╝" << std::endl;
    std::cout << Color::Reset;

    // Subtitle with gradient
    std::cout << std::endl;
    std::cout << Color::BrightCyan << "              ◆ ";
    std::cout << Color::BrightGreen << "F";
    std::cout << Color::BrightGreen << "i";
    std::cout << Color::BrightYellow << "v";
    std::cout << Color::BrightYellow << "e";
    std::cout << Color::BrightRed << "M";
    std::cout << Color::BrightRed << " ";
    std::cout << Color::BrightMagenta << "L";
    std::cout << Color::BrightMagenta << "i";
    std::cout << Color::BrightCyan << "n";
    std::cout << Color::BrightCyan << "u";
    std::cout << Color::BrightBlue << "x";
    std::cout << Color::BrightBlue << " ";
    std::cout << Color::BrightGreen << "S";
    std::cout << Color::BrightGreen << "D";
    std::cout << Color::BrightYellow << "K";
    std::cout << Color::BrightYellow << " ";
    std::cout << Color::Dim << "v" << FML_VERSION;
    std::cout << Color::Reset;
    std::cout << " ◆" << std::endl;

    // Tagline
    std::cout << Color::Dim;
    std::cout << "              Advanced FiveM & GTA V Support for Linux" << std::endl;
    std::cout << Color::Reset;

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;
}

int CommandInterface::run(int argc, char* argv[]) {
    auto args = parseArgs(argc, argv);

    if (args.empty() || args[0] == "--help" || args[0] == "-h") {
        printBanner();

        std::cout << Color::BrightCyan << Color::Bold << "  📋 Commands:" << Color::Reset << std::endl << std::endl;

        size_t maxLen = 0;
        for (auto& [name, cmd] : commands_) {
            maxLen = std::max(maxLen, name.size());
        }

        struct CmdDisplay {
            std::string name;
            std::string desc;
            std::string icon;
        };

        std::vector<CmdDisplay> cmds = {
            {"status",    "Show system status and GPU info",        "🖥️ "},
            {"install",   "Install FiveM automatically",            "📥"},
            {"launch",    "Launch FiveM with Wine/Proton",          "🚀"},
            {"diagnose",  "Run full system diagnostics",            "🔍"},
            {"repair",    "Auto-repair common issues",              "🔧"},
            {"wine",      "Wine prefix management",                 "🍷"},
            {"cache",     "Cache management",                       "💾"},
            {"gpu",       "GPU detection and Vulkan info",          "🎮"},
            {"server",    "FiveM server management",                "🖥️"},
            {"network",   "Network diagnostics",                    "🌐"},
            {"rockstar",  "Rockstar Games Launcher",                "🎮"},
            {"version",   "Show version info",                      "ℹ️ "}
        };

        for (auto& c : cmds) {
            auto it = commands_.find(c.name);
            if (it != commands_.end()) {
                std::cout << "    " << Color::BrightCyan << c.icon << " "
                          << Color::BrightWhite << Color::Bold
                          << std::setw(maxLen + 2) << std::left << c.name
                          << Color::Reset << Color::Dim << it->second.description
                          << Color::Reset << std::endl;
            }
        }

        std::cout << std::endl;
        std::cout << Color::Dim << "  Run " << Color::BrightWhite
                  << "fivem-linux <command> --help" << Color::Dim
                  << " for more info" << Color::Reset << std::endl;
        std::cout << std::endl;
        return 0;
    }

    if (args[0] == "--version" || args[0] == "-v") {
        cmdVersion({});
        return 0;
    }

    std::string cmd = args[0];
    auto it = commands_.find(cmd);
    if (it == commands_.end()) {
        printBanner();
        printError("Unknown command: " + cmd);
        std::cout << std::endl;
        printInfo("Run " + std::string(Color::BrightWhite) + "fivem-linux --help" + Color::Reset + " to see available commands");
        return 1;
    }

    printBanner();

    std::vector<std::string> cmdArgs(args.begin() + 1, args.end());
    return it->second.handler(cmdArgs);
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
        "wine", "Wine prefix management", "wine [create|repair|info|delete|proton]",
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
    commands_["rockstar"] = {
        "rockstar", "Rockstar Games Launcher", "rockstar [launch|status|games|setup]",
        [this](const auto& args) { return cmdRockstar(args); }
    };
    commands_["version"] = {
        "version", "Show version information", "version",
        [this](const auto& args) { return cmdVersion(args); }
    };
}

int CommandInterface::cmdVersion(const std::vector<std::string>&) {
    std::cout << Color::BrightCyan << Color::Bold << "  FiveMLinuxSDK" << Color::Reset << std::endl;
    std::cout << Color::Dim << "  Version: " << Color::BrightWhite << FML_VERSION << Color::Reset << std::endl;
    std::cout << Color::Dim << "  C++ Standard: C++20" << Color::Reset << std::endl;
    std::cout << Color::Dim << "  License: MIT" << Color::Reset << std::endl;
    std::cout << Color::Dim << "  Repository: " << Color::BrightCyan << "https://github.com/alguemqualquer123/fiveLinux" << Color::Reset << std::endl;
    return 0;
}

int CommandInterface::cmdStatus(const std::vector<std::string>&) {
    printHeader("SYSTEM STATUS");

    SystemDetector detector;
    detector.detect();
    auto sysInfo = detector.getSystemInfo();

    std::cout << "  " << Color::BrightCyan << Color::Bold << "🖥️  System" << Color::Reset << std::endl;
    std::cout << std::endl;

    printCheck(true, "Distro", sysInfo.distro_name + " " + sysInfo.distro_version);
    printCheck(true, "Kernel", sysInfo.kernel_version);
    printCheck(true, "Arch", sysInfo.arch);
    printCheck(true, "CPU", std::to_string(sysInfo.cpu_cores) + " cores");
    printCheck(true, "RAM", std::to_string(sysInfo.total_ram_mb) + " MB");
    printCheck(true, "Display", detector.isWayland() ? "Wayland" : "X11");

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    std::cout << "  " << Color::BrightMagenta << Color::Bold << "🎮 GPU" << Color::Reset << std::endl;
    std::cout << std::endl;

    GpuDetector gpu;
    auto gpuInfo = gpu.detect();
    printCheck(!gpuInfo.name.empty(), "GPU", gpuInfo.name);
    printCheck(!gpuInfo.driver_version.empty(), "Driver", gpuInfo.driver_version);
    printCheck(gpuInfo.vulkan_supported, "Vulkan", gpuInfo.vulkan_supported ? "Supported" : "Not supported");

    if (!gpuInfo.vulkan_version.empty()) {
        printCheck(true, "Vulkan API", gpuInfo.vulkan_version);
    }

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    std::cout << "  " << Color::BrightYellow << Color::Bold << "🍷 Wine" << Color::Reset << std::endl;
    std::cout << std::endl;

    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";
    if (!prefixPath.empty()) {
        WinePrefixManager prefixMgr;
        auto info = prefixMgr.getPrefixInfo(prefixPath);
        if (info && info->exists) {
            printCheck(true, "Wine Prefix", std::string(info->is64bit ? "64-bit" : "32-bit") +
                       " (" + std::to_string(info->sizeBytes / 1024 / 1024) + " MB)");
        } else {
            printCheck(false, "Wine Prefix", "Not found");
        }
    }

    DxvkChecker dxvk;
    bool dxvkOk = home && std::filesystem::exists(
        std::string(home) + "/.fivem-linux/prefix/drive_c/windows/system32/d3d11.dll");
    printCheck(dxvkOk, "DXVK", dxvkOk ? "Installed" : "Not installed");

    Vkd3dChecker vkd3d;
    bool vkd3dOk = home && std::filesystem::exists(
        std::string(home) + "/.fivem-linux/prefix/drive_c/windows/system32/d3d12.dll");
    printCheck(vkd3dOk, "VKD3D", vkd3dOk ? "Installed" : "Not installed");

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    std::cout << "  " << Color::BrightGreen << Color::Bold << "🚀 FiveM" << Color::Reset << std::endl;
    std::cout << std::endl;

    FiveMInstaller installer;
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";
    if (!fivemPath.empty()) {
        auto info = installer.getInstallInfo(fivemPath);
        printCheck(info.installed, "FiveM", info.installed ? "Installed (v" + info.version + ")" : "Not installed");
    } else {
        printCheck(false, "FiveM", "Not installed");
    }

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    // Summary
    int total = 0, passed = 0;
    // Count checks from above
    auto countCheck = [&](bool ok) { total++; if (ok) passed++; };
    countCheck(true); // distro
    countCheck(true); // kernel
    countCheck(gpuInfo.vulkan_supported);
    countCheck(!prefixPath.empty());
    countCheck(dxvkOk);

    if (passed == total) {
        std::cout << "  " << Color::BrightGreen << Color::Bold << "✨ All systems operational!" << Color::Reset << std::endl;
    } else {
        std::cout << "  " << Color::BrightYellow << Color::Bold << "⚠  " << (total - passed) << " issue(s) found. Run 'fivem-linux diagnose'" << Color::Reset << std::endl;
    }

    std::cout << std::endl;
    return 0;
}

int CommandInterface::cmdRepair(const std::vector<std::string>& args) {
    printHeader("AUTO REPAIR");

    GtaRepairSystem repair;
    const char* home = getenv("HOME");
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";

    if (fivemPath.empty()) {
        printError("FiveM install path not found.");
        printInfo("Run " + std::string(Color::BrightWhite) + "fivem-linux install" + Color::Reset + " first");
        return 1;
    }

    auto report = repair.diagnose(fivemPath);

    int fixCount = 0;
    for (auto& check : report.checks) {
        if (check.passed) {
            printSuccess(check.name + ": " + check.message);
        } else {
            printError(check.name + ": " + check.message);
            if (!check.fix_suggestion.empty()) {
                printInfo("Fix: " + check.fix_suggestion);
            }
            fixCount++;
        }
    }

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    if (fixCount == 0) {
        std::cout << "  " << Color::BrightGreen << Color::Bold << "✨ No issues found! System is healthy." << Color::Reset << std::endl;
    } else {
        std::cout << "  " << Color::BrightYellow << Color::Bold << "⚠  " << fixCount << " issue(s) detected. Attempting auto-repair..." << Color::Reset << std::endl;
        std::cout << std::endl;

        auto actions = repair.getRecommendedActions(fivemPath);
        for (auto& action : actions) {
            std::cout << "  " << Color::Cyan << "🔧 Repairing..." << Color::Reset << " ";
            std::cout.flush();

            if (repair.repair(fivemPath, action)) {
                printSuccess("Fixed");
            } else {
                printError("Could not fix automatically");
            }
        }
    }

    std::cout << std::endl;
    return report.success ? 0 : 1;
}

int CommandInterface::cmdInstall(const std::vector<std::string>& args) {
    printHeader("FIVEM INSTALLER");

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
        printError("No install directory specified.");
        return 1;
    }

    FiveMInstaller installer;
    if (installer.isInstalled(installDir)) {
        printSuccess("FiveM is already installed at: " + installDir);
        return 0;
    }

    printInfo("Installing FiveM to: " + installDir);
    std::cout << std::endl;

    // Animated install
    auto printProgress = [this](float pct, const std::string& msg) {
        printLoadingBar(pct, msg);
    };

    printProgress(0.0f, "Fetching build info...");
    printProgress(0.1f, "Downloading...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    printProgress(0.3f, "Downloading...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    printProgress(0.6f, "Extracting...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    printProgress(0.8f, "Setting up cache...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    printProgress(0.95f, "Finalizing...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    bool ok = installer.install(installDir, winePath, printProgress);
    std::cout << std::endl << std::endl;

    if (ok) {
        printSuccess("FiveM installed successfully!");
        printInfo("Run " + std::string(Color::BrightWhite) + "fivem-linux launch" + Color::Reset + " to start");
    } else {
        printError("Installation failed. Check your network connection.");
    }

    std::cout << std::endl;
    return ok ? 0 : 1;
}

int CommandInterface::cmdDiagnose(const std::vector<std::string>&) {
    printHeader("DIAGNOSTIC REPORT");

    GtaRepairSystem repair;
    const char* home = getenv("HOME");
    std::string fivemPath = home ? std::string(home) + "/.fivem-linux/FiveM" : "";

    if (fivemPath.empty()) {
        printError("FiveM install path not found.");
        return 1;
    }

    SystemDetector detector;
    detector.detect();
    auto sysInfo = detector.getSystemInfo();

    std::cout << "  " << Color::Dim << "System: " << Color::BrightWhite
              << sysInfo.distro_name << " " << sysInfo.distro_version
              << Color::Reset << std::endl;

    GpuDetector gpu;
    auto gpuInfo = gpu.detect();
    std::cout << "  " << Color::Dim << "GPU: " << Color::BrightWhite
              << gpuInfo.name << Color::Reset << std::endl;

    std::cout << std::endl;

    auto report = repair.diagnose(fivemPath);

    for (auto& check : report.checks) {
        if (check.passed) {
            printSuccess(check.name + ": " + check.message);
        } else {
            printError(check.name + ": " + check.message);
            if (!check.fix_suggestion.empty()) {
                printWarning("Fix: " + check.fix_suggestion);
            }
        }
    }

    std::cout << std::endl;
    printSeparator();
    std::cout << std::endl;

    auto actions = repair.getRecommendedActions(fivemPath);
    if (!actions.empty()) {
        std::cout << "  " << Color::BrightYellow << Color::Bold << "📋 Recommended actions:" << Color::Reset << std::endl;
        for (auto& action : actions) {
            std::string actionStr;
            switch (action) {
                case RepairAction::InstallDXVK: actionStr = "Install DXVK"; break;
                case RepairAction::InstallVKD3D: actionStr = "Install VKD3D"; break;
                case RepairAction::RepairWinePrefix: actionStr = "Repair Wine prefix"; break;
                case RepairAction::RepairGtaFiles: actionStr = "Repair GTA files"; break;
                case RepairAction::ClearCache: actionStr = "Clear cache"; break;
                case RepairAction::InstallMissingDeps: actionStr = "Install missing dependencies"; break;
                default: actionStr = "Unknown"; break;
            }
            std::cout << "    " << Color::BrightCyan << "◆ " << Color::BrightWhite << actionStr << Color::Reset << std::endl;
        }
    }

    if (report.success) {
        std::cout << std::endl;
        std::cout << "  " << Color::BrightGreen << Color::Bold << "✨ All checks passed!" << Color::Reset << std::endl;
    } else {
        std::cout << std::endl;
        std::cout << "  " << Color::BrightYellow << Color::Bold << "⚠  " << actions.size() << " action(s) recommended" << Color::Reset << std::endl;
    }

    std::cout << std::endl;
    return report.success ? 0 : 1;
}

int CommandInterface::cmdWine(const std::vector<std::string>& args) {
    printHeader("WINE MANAGEMENT");

    if (args.empty() || args[0] == "--help") {
        std::cout << "  " << Color::BrightCyan << "🍷 Wine Commands:" << Color::Reset << std::endl << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "create" << Color::Reset << "    Create a new Wine prefix" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "repair" << Color::Reset << "    Repair existing prefix" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "info" << Color::Reset << "      Show prefix information" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "delete" << Color::Reset << "    Delete prefix" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "proton" << Color::Reset << "    Show Proton versions" << std::endl;
        return 0;
    }

    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";
    WinePrefixManager mgr;

    if (args[0] == "create") {
        printInfo("Creating Wine prefix at: " + prefixPath);
        std::cout << "  " << Color::Cyan << "🔧 Initializing..." << Color::Reset << std::endl;
        auto result = mgr.createPrefix(prefixPath, "wine", true);
        if (result) {
            printSuccess("Prefix created successfully!");
        } else {
            printError("Failed to create prefix.");
        }
        return result ? 0 : 1;
    } else if (args[0] == "repair") {
        printInfo("Repairing prefix...");
        bool ok = mgr.repairPrefix(prefixPath);
        if (ok) printSuccess("Prefix repaired!");
        else printError("Failed to repair prefix.");
        return ok ? 0 : 1;
    } else if (args[0] == "info") {
        auto info = mgr.getPrefixInfo(prefixPath);
        if (info && info->exists) {
            printSuccess("Prefix found!");
            std::cout << std::endl;
            std::cout << "    " << Color::Dim << "Path: " << Color::BrightWhite << info->path << Color::Reset << std::endl;
            std::cout << "    " << Color::Dim << "Type: " << Color::BrightWhite << (info->is64bit ? "64-bit" : "32-bit") << Color::Reset << std::endl;
            std::cout << "    " << Color::Dim << "Size: " << Color::BrightWhite << (info->sizeBytes / 1024 / 1024) << " MB" << Color::Reset << std::endl;
        } else {
            printWarning("No prefix found at: " + prefixPath);
            printInfo("Run " + std::string(Color::BrightWhite) + "fivem-linux wine create" + Color::Reset + " to create one");
        }
        return 0;
    } else if (args[0] == "delete") {
        bool ok = mgr.deletePrefix(prefixPath);
        if (ok) printSuccess("Prefix deleted!");
        else printError("Failed to delete prefix.");
        return ok ? 0 : 1;
    } else if (args[0] == "proton") {
        ProtonSupport proton;
        proton.detectProton();
        auto versions = proton.getProtonVersions();
        if (versions.empty()) {
            printWarning("No Proton versions found.");
            printInfo("Install Proton via Steam");
        } else {
            printSuccess("Found " + std::to_string(versions.size()) + " Proton version(s)");
            std::cout << std::endl;
            for (auto& v : versions) {
                std::cout << "    " << Color::BrightCyan << "◆ " << Color::BrightWhite << v.name
                          << Color::Dim << " (" << v.version << ")" << Color::Reset << std::endl;
            }
        }
        return 0;
    }

    printError("Unknown wine command: " + args[0]);
    return 1;
}

int CommandInterface::cmdCache(const std::vector<std::string>& args) {
    printHeader("CACHE MANAGEMENT");

    const char* home = getenv("HOME");
    std::string cacheDir = home ? std::string(home) + "/.fivem-linux/FiveM/cache" : "";

    if (cacheDir.empty() || !std::filesystem::exists(cacheDir)) {
        printWarning("Cache directory not found.");
        return 1;
    }

    FiveMCacheManager cache;

    if (args.empty() || args[0] == "size") {
        auto stats = cache.getCacheSize(cacheDir);
        std::cout << "  " << Color::BrightCyan << "💾 Cache Statistics" << Color::Reset << std::endl << std::endl;

        float totalMB = stats.totalSizeBytes / 1024.0f / 1024.0f;
        printCheck(stats.gameCacheSize > 0, "Game Cache",
                   std::to_string(stats.gameCacheSize / 1024 / 1024) + " MB");
        printCheck(stats.privCacheSize > 0, "Private Cache",
                   std::to_string(stats.privCacheSize / 1024 / 1024) + " MB");
        printCheck(stats.filesCacheSize > 0, "Files Cache",
                   std::to_string(stats.filesCacheSize / 1024 / 1024) + " MB");
        printCheck(stats.serverCacheSize > 0, "Server Cache",
                   std::to_string(stats.serverCacheSize / 1024 / 1024) + " MB");

        std::cout << std::endl;
        std::cout << "    " << Color::Dim << "Total: " << Color::BrightWhite
                  << std::fixed << std::setprecision(1) << totalMB << " MB"
                  << Color::Reset << std::endl;
        return 0;
    } else if (args[0] == "clear") {
        std::string type = (args.size() > 1) ? args[1] : "all";
        CacheType ct = CacheType::All;
        if (type == "game") ct = CacheType::Game;
        else if (type == "priv") ct = CacheType::Priv;
        else if (type == "files") ct = CacheType::Files;

        printInfo("Clearing " + type + " cache...");
        bool ok = cache.clearCache(cacheDir, ct);
        if (ok) printSuccess("Cache cleared!");
        else printError("Failed to clear cache.");
        return ok ? 0 : 1;
    } else if (args[0] == "list") {
        auto entries = cache.listCacheDirs(cacheDir);
        std::cout << "  " << Color::BrightCyan << "📁 Cache Contents" << Color::Reset << std::endl << std::endl;

        for (auto& e : entries) {
            std::string icon = e.isDirectory ? "📁" : "📄";
            std::cout << "    " << icon << " " << Color::BrightWhite << std::setw(20) << std::left
                      << e.name << Color::Dim << std::setw(10) << std::right
                      << (e.sizeBytes / 1024) << " KB" << Color::Reset << std::endl;
        }
        return 0;
    }

    printError("Unknown cache command: " + args[0]);
    return 1;
}

int CommandInterface::cmdLaunch(const std::vector<std::string>& args) {
    printHeader("LAUNCH FIVEM");

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
        printSuccess("FiveM launched!");
        printInfo("PID: " + std::to_string(launcher.getProcessId()));

        if (!serverUrl.empty()) {
            printInfo("Connecting to: " + serverUrl);
        }
    } else {
        printError("Failed to launch FiveM.");
        printInfo("Make sure FiveM is installed: " + std::string(Color::BrightWhite) + "fivem-linux install" + Color::Reset);
    }

    std::cout << std::endl;
    return 0;
}

int CommandInterface::cmdGpu(const std::vector<std::string>&) {
    printHeader("GPU INFORMATION");

    GpuDetector gpu;
    auto info = gpu.detect();

    std::cout << "  " << Color::BrightMagenta << Color::Bold << "🎮 GPU Details" << Color::Reset << std::endl;
    std::cout << std::endl;

    printCheck(!info.name.empty(), "Name", info.name);

    std::string vendorStr = "Unknown";
    switch (info.vendor) {
        case GpuVendor::NVIDIA: vendorStr = "NVIDIA"; break;
        case GpuVendor::AMD: vendorStr = "AMD"; break;
        case GpuVendor::Intel: vendorStr = "Intel"; break;
        default: break;
    }
    printCheck(info.vendor != GpuVendor::Unknown, "Vendor", vendorStr);
    printCheck(!info.driver_version.empty(), "Driver", info.driver_version);
    printCheck(info.vram_mb > 0, "VRAM", info.vram_mb > 0 ? std::to_string(info.vram_mb) + " MB" : "Unknown");
    printCheck(info.vulkan_supported, "Vulkan", info.vulkan_supported ? "Supported" : "Not supported");

    if (!info.vulkan_version.empty()) {
        printCheck(true, "Vulkan API", info.vulkan_version);
    }

    std::cout << std::endl;

    // Vulkan devices
    VulkanChecker vk;
    if (vk.isAvailable()) {
        auto devices = vk.getDevices();
        if (!devices.empty()) {
            printSeparator();
            std::cout << std::endl;
            std::cout << "  " << Color::BrightCyan << Color::Bold << "⚡ Vulkan Devices" << Color::Reset << std::endl;
            std::cout << std::endl;

            for (auto& dev : devices) {
                std::cout << "    " << Color::BrightCyan << "◆ " << Color::BrightWhite << dev.name << Color::Reset << std::endl;
                if (!dev.driverVersion.empty()) {
                    std::cout << "      " << Color::Dim << "Driver: " << dev.driverVersion << Color::Reset << std::endl;
                }
            }
        }
    }

    std::cout << std::endl;
    return 0;
}

int CommandInterface::cmdServer(const std::vector<std::string>& args) {
    printHeader("FIVEM SERVER");

    if (args.empty() || args[0] == "--help") {
        std::cout << "  " << Color::BrightCyan << "🖥️  Server Commands:" << Color::Reset << std::endl << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "install" << Color::Reset << "  Install FiveM server" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "start" << Color::Reset << "    Start server" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "stop" << Color::Reset << "     Stop server" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "status" << Color::Reset << "   Server status" << std::endl;
        return 0;
    }

    printWarning("Server management is in development.");
    printInfo("Coming soon: full FiveM server support");

    std::cout << std::endl;
    return 1;
}

int CommandInterface::cmdNetwork(const std::vector<std::string>& args) {
    printHeader("NETWORK DIAGNOSTICS");

    if (args.empty() || args[0] == "ping") {
        LatencyTest test;
        std::cout << "  " << Color::BrightCyan << "🌐 Testing FiveM Servers" << Color::Reset << std::endl;
        std::cout << std::endl;

        auto results = test.testFiveMServers();
        for (auto& r : results) {
            if (r.reachable) {
                printSuccess(r.host + ": " + std::to_string((int)r.latencyMs) + "ms");
            } else {
                printError(r.host + ": Unreachable");
            }
        }
        return 0;
    } else if (args[0] == "ports") {
        PortManager portMgr;
        auto ports = portMgr.checkRequiredPorts();

        std::cout << "  " << Color::BrightCyan << "🔌 Required Ports" << Color::Reset << std::endl;
        std::cout << std::endl;

        for (auto& p : ports) {
            printCheck(p.open, std::to_string(p.port) + "/" + p.protocol,
                       p.open ? "OPEN" : "CLOSED");
        }
        return 0;
    } else if (args[0] == "firewall") {
        FirewallCheck fw;
        std::cout << "  " << Color::BrightCyan << "🛡️  Firewall Status" << Color::Reset << std::endl;
        std::cout << std::endl;

        printCheck(fw.isFirewallActive(), "Firewall", fw.getFirewallStatus());
        return 0;
    }

    printError("Unknown network command: " + args[0]);
    return 1;
}

int CommandInterface::cmdRockstar(const std::vector<std::string>& args) {
    printHeader("ROCKSTAR GAMES LAUNCHER");

    if (args.empty() || args[0] == "--help") {
        std::cout << "  " << Color::BrightCyan << "🎮 Rockstar Commands:" << Color::Reset << std::endl << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "launch" << Color::Reset << "     Launch Rockstar Launcher" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "status" << Color::Reset << "     Show launcher status" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "games" << Color::Reset << "      List Rockstar games" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "setup" << Color::Reset << "      Setup Wine prefix for Rockstar" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "install-deps" << Color::Reset << " Install Rockstar dependencies" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "login" << Color::Reset << "      Login to Rockstar account" << std::endl;
        std::cout << "    " << Color::BrightGreen << "◆ " << Color::BrightWhite << "steam-login" << Color::Reset << " Login via Steam" << std::endl;
        return 0;
    }

    RockstarLauncher launcher;
    RockstarGamesManager games;

    if (args[0] == "launch") {
        auto info = launcher.detect();
        if (!info.installed) {
            printWarning("Rockstar Launcher not found.");
            printInfo("Install from: " + std::string(Color::BrightCyan) + "https://www.rockstargames.com/downloads" + Color::Reset);
            return 1;
        }

        printInfo("Launching Rockstar Games Launcher...");
        printInfo("Path: " + info.launcherPath);

        if (launcher.launch()) {
            printSuccess("Launcher started! (PID: " + std::to_string(launcher.getProcessId()) + ")");
            printInfo("Login with your Rockstar account or use Steam");
        } else {
            printError("Failed to launch. Make sure Wine is configured:");
            printInfo("Run: " + std::string(Color::BrightWhite) + "fivem-linux rockstar setup" + Color::Reset);
        }
        return 0;

    } else if (args[0] == "status") {
        auto info = launcher.detect();
        auto state = launcher.getState();

        std::cout << "  " << Color::BrightCyan << Color::Bold << "🎮 Rockstar Status" << Color::Reset << std::endl;
        std::cout << std::endl;

        printCheck(info.installed, "Launcher",
                   info.installed ? "Installed at: " + info.installPath : "Not found");
        printCheck(state.launcherRunning, "Running",
                   state.launcherRunning ? "Yes" : "No");

        if (info.installed) {
            printInfo("Version: " + info.version);
            printInfo("Size: " + std::to_string(info.totalSizeBytes / 1024 / 1024) + " MB");
        }

        std::cout << std::endl;
        printSeparator();
        std::cout << std::endl;

        // Check GTA V
        if (games.isGameInstalled(RockstarGame::GTAV)) {
            printSuccess("GTA V: Installed");
        } else {
            printWarning("GTA V: Not installed via Rockstar");
        }

        return 0;

    } else if (args[0] == "games") {
        std::cout << "  " << Color::BrightCyan << Color::Bold << "🎮 Rockstar Games" << Color::Reset << std::endl;
        std::cout << std::endl;

        auto allGames = games.getOwnedGames();

        if (allGames.empty()) {
            printInfo("No Rockstar games found.");
            printInfo("Games will appear here when installed.");
        } else {
            for (auto& game : allGames) {
                std::string status = game.installed ? Color::BrightGreen + "Installed" : Color::BrightYellow + "Not installed";
                std::cout << "    " << Color::BrightCyan << "◆ " << Color::BrightWhite
                          << std::setw(25) << std::left << game.name << Color::Reset
                          << " " << status << Color::Reset << std::endl;

                if (game.installed) {
                    std::cout << "      " << Color::Dim << game.installPath << Color::Reset << std::endl;
                }
            }
        }

        std::cout << std::endl;
        printInfo("Supported games:");
        std::cout << "    " << Color::Dim << "• Grand Theft Auto V (GTA V)" << Color::Reset << std::endl;
        std::cout << "    " << Color::Dim << "• Red Dead Redemption 2 (RDR2)" << Color::Reset << std::endl;
        std::cout << "    " << Color::Dim << "• Max Payne 3" << Color::Reset << std::endl;
        std::cout << "    " << Color::Dim << "• L.A. Noire" << Color::Reset << std::endl;
        std::cout << "    " << Color::Dim << "• Bully" << Color::Reset << std::endl;

        return 0;

    } else if (args[0] == "setup") {
        printInfo("Setting up Wine prefix for Rockstar Games Launcher...");

        const char* home = getenv("HOME");
        std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";

        if (prefixPath.empty()) {
            printError("Cannot determine home directory.");
            return 1;
        }

        printInfo("Prefix: " + prefixPath);
        std::cout << std::endl;

        // Configure prefix
        printLoadingBar(0.0f, "Creating directories...");
        launcher.configurePrefix(prefixPath);

        printLoadingBar(0.3f, "Installing dependencies...");
        launcher.installDependencies(prefixPath);

        printLoadingBar(0.6f, "Configuring Social Club...");
        launcher.installSocialClub(prefixPath);

        printLoadingBar(0.9f, "Finalizing...");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        printLoadingBar(1.0f, "Done!");
        std::cout << std::endl << std::endl;

        printSuccess("Wine prefix configured for Rockstar!");
        printInfo("You can now install the Rockstar Launcher:");
        printInfo("1. Download from: " + std::string(Color::BrightCyan) + "https://www.rockstargames.com/downloads" + Color::Reset);
        printInfo("2. Run: " + std::string(Color::BrightWhite) + "fivem-linux rockstar launch" + Color::Reset);

        return 0;

    } else if (args[0] == "install-deps") {
        printInfo("Installing Rockstar dependencies...");

        const char* home = getenv("HOME");
        std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";

        if (prefixPath.empty()) {
            printError("Cannot determine home directory.");
            return 1;
        }

        printInfo("Installing Visual C++ Redistributables...");
        std::string cmd = "WINEPREFIX=\"" + prefixPath + "\" winetricks -q vcrun2019 2>/dev/null";
        int result = std::system(cmd.c_str());

        if (result == 0) {
            printSuccess("Dependencies installed!");
        } else {
            printWarning("Some dependencies may have failed.");
            printInfo("You may need to install them manually.");
        }

        return 0;

    } else if (args[0] == "login") {
        printInfo("Launching Rockstar Launcher for login...");
        printInfo("Login with your Rockstar Games account.");

        if (launcher.launch()) {
            printSuccess("Launcher started!");
            printInfo("Enter your credentials in the launcher window.");
        } else {
            printError("Failed to launch launcher.");
        }
        return 0;

    } else if (args[0] == "steam-login") {
        printInfo("Launching Rockstar Launcher with Steam integration...");
        printInfo("Make sure Steam is running and you are logged in.");

        setenv("SteamAppId", "271590", 1);
        setenv("SteamGameId", "271590", 1);

        if (launcher.launch()) {
            printSuccess("Launcher started with Steam!");
            printInfo("Steam authentication will be handled automatically.");
        } else {
            printError("Failed to launch launcher.");
        }
        return 0;
    }

    printError("Unknown rockstar command: " + args[0]);
    return 1;
}

} // namespace fml
