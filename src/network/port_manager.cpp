#include "port_manager.h"

#include <sstream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace fml {

std::vector<uint16_t> PortManager::getFiveMRequiredPorts() const {
    return {30120, 30121, 30122};
}

bool PortManager::checkTcpPort(uint16_t port) const {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return result == 0;
}

bool PortManager::checkUdpPort(uint16_t port) const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return false;

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    char buf[1] = {0};
    sendto(sock, buf, 0, 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return true;
}

std::vector<PortInfo> PortManager::checkRequiredPorts() const {
    std::vector<PortInfo> results;
    auto required = getFiveMRequiredPorts();

    for (auto port : required) {
        PortInfo info{};
        info.port = port;
        info.protocol = "tcp/udp";
        info.open = checkTcpPort(port) || checkUdpPort(port);
        info.required = true;
        info.service = "FiveM Game Server";
        results.push_back(info);
    }

    return results;
}

bool PortManager::openFiveMPorts(uint32_t gamePort) const {
    std::string portStr = std::to_string(gamePort);
    std::string cmd =
        "ufw allow " + portStr + "/tcp 2>/dev/null && "
        "ufw allow " + portStr + "/udp 2>/dev/null && "
        "iptables -A INPUT -p tcp --dport " + portStr + " -j ACCEPT 2>/dev/null && "
        "iptables -A INPUT -p udp --dport " + portStr + " -j ACCEPT 2>/dev/null";
    return std::system(cmd.c_str()) == 0;
}

std::vector<uint16_t> PortManager::getOpenPorts() const {
    std::vector<uint16_t> openPorts;
    auto pipe = popen("ss -tlnp 2>/dev/null | awk 'NR>1{print $4}' | grep -oP ':\\K[0-9]+'", "r");
    if (!pipe) return openPorts;

    char buffer[64];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string portStr = buffer;
        portStr.erase(std::remove(portStr.begin(), portStr.end(), '\n'), portStr.end());
        try {
            openPorts.push_back(static_cast<uint16_t>(std::stoi(portStr)));
        } catch (...) {}
    }
    pclose(pipe);
    return openPorts;
}

bool PortManager::isPortAvailable(uint16_t port) const {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int result = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return result == 0;
}

bool PortManager::isPortInUse(uint16_t port) const {
    return !isPortAvailable(port);
}

} // namespace fml
