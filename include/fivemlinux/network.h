#pragma once

#include <string>

namespace fivemlinux {
namespace network {

class Network {
public:
    Network();
    ~Network();

    bool connect(const std::string& address, int port);
    void disconnect();
};

} // namespace network
} // namespace fivemlinux
