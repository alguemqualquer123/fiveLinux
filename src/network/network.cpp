#include "fivemlinux/network.h"

namespace fivemlinux {
namespace network {

Network::Network() = default;
Network::~Network() = default;

bool Network::connect(const std::string& address, int port) {
    return true;
}

void Network::disconnect() {
}

} // namespace network
} // namespace fivemlinux
