#pragma once

#include <string>
#include <vector>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

struct VulkanDeviceInfo {
    std::string name;
    std::string driverVersion;
    std::string apiVersion;
    uint64_t vramBytes;
    bool discrete;
};

class VulkanChecker {
public:
    VulkanChecker() = default;

    bool isAvailable() const;
    std::string getVersion() const;
    std::vector<std::string> getExtensions() const;
    std::vector<VulkanDeviceInfo> getDevices() const;
    std::string getDriverVersion() const;
    bool isNvidia() const;
    bool isAmd() const;
    bool isIntel() const;

private:
    std::string runVulkanInfo(const std::string& args) const;
    std::vector<std::string> parseExtensions(const std::string& output) const;
    std::vector<VulkanDeviceInfo> parseDevices(const std::string& output) const;
};

} // namespace fml
