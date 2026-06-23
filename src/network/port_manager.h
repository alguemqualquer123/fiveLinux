#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

struct PortInfo {
    uint16_t port;
    std::string protocol;
    bool open;
    bool required;
    std::string service;
};

class PortManager {
public:
    PortManager() = default;

    std::vector<PortInfo> checkRequiredPorts() const;
    bool openFiveMPorts(uint32_t gamePort = 30120) const;
    std::vector<uint16_t> getOpenPorts() const;
    bool isPortAvailable(uint16_t port) const;
    bool isPortInUse(uint16_t port) const;

private:
    bool checkTcpPort(uint16_t port) const;
    bool checkUdpPort(uint16_t port) const;
    std::vector<uint16_t> getFiveMRequiredPorts() const;
};

} // namespace fml
