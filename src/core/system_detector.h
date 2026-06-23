#pragma once

#include <string>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

class SystemDetector {
public:
    SystemDetector();

    void detect();
    const SystemInfo& getSystemInfo() const;
    uint64_t getUptime() const;
    bool isWayland() const;
    bool hasSudo() const;

private:
    void readOsRelease();
    void readUname();
    void readCpuInfo();
    void readMemInfo();
    void detectDesktop();
    void detectSudo();

    SystemInfo info_;
    bool wayland_ = false;
    bool sudo_ = false;
};

} // namespace fml