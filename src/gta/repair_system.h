#pragma once

#include <string>
#include <vector>

#include "fivemlinux/types.h"

namespace fml {

enum class RepairAction {
    None,
    InstallDXVK,
    InstallVKD3D,
    RepairWinePrefix,
    RepairGtaFiles,
    UpdateDriver,
    InstallMissingDeps,
    ClearCache
};

class GtaRepairSystem {
public:
    GtaRepairSystem() = default;

    DiagnosticReport diagnose(const std::string& installDir) const;
    bool repair(const std::string& installDir, RepairAction action);
    bool verify(const std::string& installDir) const;
    std::vector<RepairAction> getRecommendedActions(const std::string& installDir) const;

private:
    void addCheck(DiagnosticReport& report, const std::string& name,
                  bool passed, const std::string& message,
                  const std::string& fix = "");
};

} // namespace fml
