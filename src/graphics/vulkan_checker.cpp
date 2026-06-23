#include "vulkan_checker.h"

#include <sstream>
#include <cstdlib>
#include <fstream>

namespace fml {

std::string VulkanChecker::runVulkanInfo(const std::string& args) const {
    std::string cmd = "vulkaninfo " + args + " 2>/dev/null";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    std::ostringstream result;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result << buffer;
    }
    pclose(pipe);
    return result.str();
}

bool VulkanChecker::isAvailable() const {
    int result = std::system("which vulkaninfo >/dev/null 2>&1");
    if (result != 0) return false;

    std::string output = runVulkanInfo("--summary");
    return output.find("VK_ERROR_INITIALIZATION_FAILED") == std::string::npos &&
           !output.empty();
}

std::string VulkanChecker::getVersion() const {
    std::string output = runVulkanInfo("--summary");
    size_t pos = output.find("apiVersion");
    if (pos != std::string::npos) {
        size_t eq = output.find('=', pos);
        size_t end = output.find('\n', eq);
        if (eq != std::string::npos && end != std::string::npos) {
            return output.substr(eq + 2, end - eq - 2);
        }
    }

    pos = output.find("VK_HEADER_VERSION");
    if (pos != std::string::npos) {
        size_t eq = output.find('=', pos);
        size_t end = output.find('\n', eq);
        if (eq != std::string::npos && end != std::string::npos) {
            return "header:" + output.substr(eq + 2, end - eq - 2);
        }
    }

    return "unknown";
}

std::vector<std::string> VulkanChecker::getExtensions() const {
    std::string output = runVulkanInfo("--list-extensions");
    return parseExtensions(output);
}

std::vector<std::string> VulkanChecker::parseExtensions(const std::string& output) const {
    std::vector<std::string> extensions;
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("VK_") != std::string::npos) {
            size_t start = line.find("VK_");
            size_t end = line.find(' ', start);
            if (end == std::string::npos) end = line.length();
            extensions.push_back(line.substr(start, end - start));
        }
    }

    return extensions;
}

std::vector<VulkanDeviceInfo> VulkanChecker::getDevices() const {
    std::string output = runVulkanInfo("--list-gpus");
    return parseDevices(output);
}

std::vector<VulkanDeviceInfo> VulkanChecker::parseDevices(const std::string& output) const {
    std::vector<VulkanDeviceInfo> devices;
    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("GPU") != std::string::npos && line.find(":") != std::string::npos) {
            VulkanDeviceInfo device{};
            size_t colon = line.find(':');
            device.name = line.substr(colon + 2);
            size_t paren = device.name.find('(');
            if (paren != std::string::npos) {
                device.name = device.name.substr(0, paren);
            }
            device.discrete = (line.find("discrete") != std::string::npos ||
                              line.find("Discrete") != std::string::npos);
            devices.push_back(device);
        }
    }

    return devices;
}

std::string VulkanChecker::getDriverVersion() const {
    std::string output = runVulkanInfo("--summary");
    size_t pos = output.find("driverVersion");
    if (pos != std::string::npos) {
        size_t eq = output.find('=', pos);
        size_t end = output.find('\n', eq);
        if (eq != std::string::npos && end != std::string::npos) {
            return output.substr(eq + 2, end - eq - 2);
        }
    }
    return "unknown";
}

bool VulkanChecker::isNvidia() const {
    auto devices = getDevices();
    for (auto& d : devices) {
        if (d.name.find("NVIDIA") != std::string::npos ||
            d.name.find("GeForce") != std::string::npos) return true;
    }
    return false;
}

bool VulkanChecker::isAmd() const {
    auto devices = getDevices();
    for (auto& d : devices) {
        if (d.name.find("AMD") != std::string::npos ||
            d.name.find("Radeon") != std::string::npos) return true;
    }
    return false;
}

bool VulkanChecker::isIntel() const {
    auto devices = getDevices();
    for (auto& d : devices) {
        if (d.name.find("Intel") != std::string::npos) return true;
    }
    return false;
}

} // namespace fml
