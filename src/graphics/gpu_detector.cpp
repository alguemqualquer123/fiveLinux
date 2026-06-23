#include "gpu_detector.h"

#include <sstream>
#include <cstdlib>
#include <fstream>

namespace fml {

std::vector<GpuDetector::PciDevice> GpuDetector::parseLspci() const {
    std::vector<PciDevice> devices;
    auto pipe = popen("lspci -nn 2>/dev/null", "r");
    if (!pipe) return devices;

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line = buffer;
        if (line.find("VGA") != std::string::npos ||
            line.find("3D") != std::string::npos ||
            line.find("Display") != std::string::npos) {
            PciDevice dev{};
            size_t colon = line.find(": ", line.find(": ") + 1);
            if (colon != std::string::npos) {
                dev.name = line.substr(colon + 2);
                dev.name.erase(std::remove(dev.name.begin(), dev.name.end(), '\n'), dev.name.end());
            }

            if (dev.name.find("NVIDIA") != std::string::npos ||
                dev.name.find("GeForce") != std::string::npos) {
                dev.vendor = "NVIDIA";
            } else if (dev.name.find("AMD") != std::string::npos ||
                       dev.name.find("Radeon") != std::string::npos) {
                dev.vendor = "AMD";
            } else if (dev.name.find("Intel") != std::string::npos) {
                dev.vendor = "Intel";
            }

            devices.push_back(dev);
        }
    }
    pclose(pipe);
    return devices;
}

std::string GpuDetector::getNvidiaDriverVersion() const {
    auto pipe = popen("nvidia-smi --query-gpu=driver_version --format=csv,noheader 2>/dev/null", "r");
    if (!pipe) return "";

    char buffer[64];
    std::string version;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        version = buffer;
        version.erase(std::remove(version.begin(), version.end(), '\n'), version.end());
    }
    pclose(pipe);
    return version;
}

uint64_t GpuDetector::getNvidiaVram() const {
    auto pipe = popen("nvidia-smi --query-gpu=memory.total --format=csv,noheader 2>/dev/null", "r");
    if (!pipe) return 0;

    char buffer[64];
    uint64_t vram = 0;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        std::string str = buffer;
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
        str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
        try { vram = std::stoull(str); } catch (...) {}
    }
    pclose(pipe);
    return vram;
}

std::string GpuDetector::getAmdDriverVersion() const {
    auto pipe = popen("cat /sys/class/drm/card0/device/driver/module/version 2>/dev/null", "r");
    if (!pipe) return "";

    char buffer[64];
    std::string version;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        version = buffer;
        version.erase(std::remove(version.begin(), version.end(), '\n'), version.end());
    }
    pclose(pipe);
    return version;
}

bool GpuDetector::checkOpengl() const {
    int result = std::system("glxinfo >/dev/null 2>&1");
    return result == 0;
}

std::string GpuDetector::getGlVersion() const {
    auto pipe = popen("glxinfo 2>/dev/null | grep 'OpenGL version' | head -1", "r");
    if (!pipe) return "";

    char buffer[256];
    std::string version;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        version = buffer;
        size_t pos = version.find(':');
        if (pos != std::string::npos) {
            version = version.substr(pos + 2);
        }
        version.erase(std::remove(version.begin(), version.end(), '\n'), version.end());
    }
    pclose(pipe);
    return version;
}

GpuInfo GpuDetector::detect() const {
    GpuInfo info{};
    info.vendor = GpuVendor::Unknown;

    auto devices = parseLspci();
    if (!devices.empty()) {
        info.name = devices[0].name;

        if (devices[0].vendor == "NVIDIA") {
            info.vendor = GpuVendor::NVIDIA;
            info.driver_version = getNvidiaDriverVersion();
            info.vram_mb = getNvidiaVram();
        } else if (devices[0].vendor == "AMD") {
            info.vendor = GpuVendor::AMD;
            info.driver_version = getAmdDriverVersion();
        } else if (devices[0].vendor == "Intel") {
            info.vendor = GpuVendor::Intel;
        }
    }

    auto pipe = popen("which vulkaninfo >/dev/null 2>&1", "r");
    info.vulkan_supported = (pclose(pipe) == 0);

    if (info.vulkan_supported) {
        auto vPipe = popen("vulkaninfo --summary 2>/dev/null | grep apiVersion", "r");
        if (vPipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), vPipe)) {
                std::string line = buffer;
                size_t eq = line.find('=');
                if (eq != std::string::npos) {
                    info.vulkan_version = line.substr(eq + 2);
                    info.vulkan_version.erase(
                        std::remove(info.vulkan_version.begin(), info.vulkan_version.end(), '\n'),
                        info.vulkan_version.end());
                }
            }
            pclose(vPipe);
        }
    }

    return info;
}

std::string GpuDetector::getGpuName() const {
    auto devices = parseLspci();
    return devices.empty() ? "Unknown" : devices[0].name;
}

GpuVendor GpuDetector::getVendor() const {
    auto devices = parseLspci();
    if (devices.empty()) return GpuVendor::Unknown;
    if (devices[0].vendor == "NVIDIA") return GpuVendor::NVIDIA;
    if (devices[0].vendor == "AMD") return GpuVendor::AMD;
    if (devices[0].vendor == "Intel") return GpuVendor::Intel;
    return GpuVendor::Unknown;
}

std::string GpuDetector::getDriverVersion() const {
    auto vendor = getVendor();
    if (vendor == GpuVendor::NVIDIA) return getNvidiaDriverVersion();
    if (vendor == GpuVendor::AMD) return getAmdDriverVersion();
    return "unknown";
}

uint64_t GpuDetector::getVramBytes() const {
    auto vendor = getVendor();
    if (vendor == GpuVendor::NVIDIA) return getNvidiaVram() * 1024 * 1024;
    return 0;
}

bool GpuDetector::hasNvidia() const { return getVendor() == GpuVendor::NVIDIA; }
bool GpuDetector::hasAmd() const { return getVendor() == GpuVendor::AMD; }
bool GpuDetector::hasIntel() const { return getVendor() == GpuVendor::Intel; }

} // namespace fml
