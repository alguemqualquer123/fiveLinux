#include "repair_system.h"
#include "files_checker.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

namespace fml {

void GtaRepairSystem::addCheck(DiagnosticReport& report, const std::string& name,
                                bool passed, const std::string& message,
                                const std::string& fix) const {
    DiagnosticReport::Check check{};
    check.name = name;
    check.passed = passed;
    check.message = message;
    check.fix_suggestion = fix;
    report.checks.push_back(check);
}

DiagnosticReport GtaRepairSystem::diagnose(const std::string& installDir) const {
    DiagnosticReport report{};
    report.success = true;

    GtaFilesChecker checker;
    auto missing = checker.getMissingFiles(installDir);
    auto corrupt = checker.getCorruptFiles(installDir);
    float completion = checker.getCompletionPercentage(installDir);

    report.fivem.has_gta_files = (completion > 50.0f);
    report.fivem.missing_files = missing;
    report.fivem.corrupt_files = corrupt;

    addCheck(report, "GTA Files", missing.empty() && corrupt.empty(),
             "Completion: " + std::to_string((int)completion) + "%",
             missing.empty() ? "" : "Install/re-download missing files: " + missing[0]);

    const char* home = getenv("HOME");
    std::string prefixPath = home ? std::string(home) + "/.fivem-linux/prefix" : "";
    bool prefixExists = std::filesystem::exists(prefixPath);
    addCheck(report, "Wine Prefix", prefixExists,
             prefixExists ? "Found at " + prefixPath : "Not found",
             prefixExists ? "" : "Create prefix with: fivem-linux wine create");

    bool wineAvailable = (std::system("which wine >/dev/null 2>&1") == 0);
    addCheck(report, "Wine", wineAvailable,
             wineAvailable ? "Wine is available" : "Wine not found",
             wineAvailable ? "" : "Install wine with your package manager");

    bool vulkanAvailable = (std::system("which vulkaninfo >/dev/null 2>&1") == 0);
    addCheck(report, "Vulkan", vulkanAvailable,
             vulkanAvailable ? "Vulkan available" : "Vulkan not found",
             vulkanAvailable ? "" : "Install vulkan-tools and mesa-vulkan-drivers");

    bool dxvkInstalled = false;
    if (prefixExists) {
        dxvkInstalled = std::filesystem::exists(
            std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d11.dll");
    }
    addCheck(report, "DXVK", dxvkInstalled,
             dxvkInstalled ? "DXVK installed" : "DXVK not installed in prefix",
             dxvkInstalled ? "" : "Install DXVK with: fivem-linux graphics install-dxvk");

    bool nvidiaDriver = (std::system("nvidia-smi >/dev/null 2>&1") == 0);
    bool amdgpuDriver = std::filesystem::exists("/sys/class/drm/card0/device/driver");
    addCheck(report, "GPU Driver", nvidiaDriver || amdgpuDriver,
             nvidiaDriver ? "NVIDIA driver detected" :
             (amdgpuDriver ? "AMD driver detected" : "No GPU driver detected"),
             (nvidiaDriver || amdgpuDriver) ? "" : "Install proprietary GPU drivers");

    bool networkOk = (std::system("ping -c 1 runtime.fivem.net >/dev/null 2>&1") == 0);
    addCheck(report, "Network", networkOk,
             networkOk ? "Network connectivity OK" : "Cannot reach FiveM servers",
             networkOk ? "" : "Check network connection and firewall");

    for (auto& check : report.checks) {
        if (!check.passed) {
            report.success = false;
            break;
        }
    }

    return report;
}

bool GtaRepairSystem::repair(const std::string& installDir, RepairAction action) {
    switch (action) {
        case RepairAction::InstallDXVK: {
            std::string cmd = "fivem-linux graphics install-dxvk 2>/dev/null";
            return std::system(cmd.c_str()) == 0;
        }
        case RepairAction::InstallVKD3D: {
            std::string cmd = "fivem-linux graphics install-vkd3d 2>/dev/null";
            return std::system(cmd.c_str()) == 0;
        }
        case RepairAction::RepairWinePrefix: {
            const char* home = getenv("HOME");
            if (!home) return false;
            std::string prefixPath = std::string(home) + "/.fivem-linux/prefix";
            std::string cmd = "WINEPREFIX=\"" + prefixPath + "\" wineboot -u 2>/dev/null";
            return std::system(cmd.c_str()) == 0;
        }
        case RepairAction::ClearCache: {
            std::string cmd = "fivem-linux cache clear 2>/dev/null";
            return std::system(cmd.c_str()) == 0;
        }
        case RepairAction::InstallMissingDeps: {
            std::string cmd = "fivem-linux install-deps 2>/dev/null";
            return std::system(cmd.c_str()) == 0;
        }
        default:
            return false;
    }
}

bool GtaRepairSystem::verify(const std::string& installDir) const {
    auto report = diagnose(installDir);
    return report.success;
}

std::vector<RepairAction> GtaRepairSystem::getRecommendedActions(
    const std::string& installDir) const
{
    std::vector<RepairAction> actions;
    auto report = diagnose(installDir);

    for (auto& check : report.checks) {
        if (!check.passed) {
            if (check.name == "DXVK") actions.push_back(RepairAction::InstallDXVK);
            else if (check.name == "Wine Prefix") actions.push_back(RepairAction::RepairWinePrefix);
            else if (check.name == "Vulkan") actions.push_back(RepairAction::InstallMissingDeps);
            else if (check.name == "GTA Files") actions.push_back(RepairAction::RepairGtaFiles);
        }
    }

    return actions;
}

} // namespace fml
