#pragma once

#include "fivemlinux/types.h"
#include <string>

namespace fml {

class FiveMLinuxSDK {
public:
    static FiveMLinuxSDK& instance();

    bool initialize();
    void shutdown();

    SystemInfo getSystemInfo() const;
    GpuInfo getGpuInfo() const;
    WineInfo getWineInfo() const;
    GraphicsState getGraphicsState() const;
    FiveMState getFiveMState() const;
    NetworkInfo getNetworkInfo() const;

    bool repair();
    bool diagnose();
    std::string getVersion() const;

    bool createWinePrefix(const std::string& path);
    bool installFiveM(const std::string& installDir);
    bool launchFiveM(const std::string& installDir);

private:
    FiveMLinuxSDK() = default;
    ~FiveMLinuxSDK() = default;
    FiveMLinuxSDK(const FiveMLinuxSDK&) = delete;
    FiveMLinuxSDK& operator=(const FiveMLinuxSDK&) = delete;

    bool initialized_ = false;
};

}
