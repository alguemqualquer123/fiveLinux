#pragma once

#include <string>
#include <vector>
#include <optional>

#include "fivemlinux/types.h"

namespace fml {

class GpuDetector {
public:
    GpuDetector() = default;

    GpuInfo detect() const;
    std::string getGpuName() const;
    GpuVendor getVendor() const;
    std::string getDriverVersion() const;
    uint64_t getVramBytes() const;
    bool hasNvidia() const;
    bool hasAmd() const;
    bool hasIntel() const;

private:
    struct PciDevice {
        std::string name;
        std::string vendor;
        std::string driver;
        std::string path;
    };

    std::vector<PciDevice> parseLspci() const;
    std::string getNvidiaDriverVersion() const;
    uint64_t getNvidiaVram() const;
    std::string getAmdDriverVersion() const;
    bool checkOpengl() const;
    std::string getGlVersion() const;
};

} // namespace fml
