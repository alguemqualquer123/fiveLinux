#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

enum class FirewallType {
    None,
    Iptables,
    Nftables,
    Ufw,
    Firewalld,
    Unknown
};

struct FirewallRule {
    std::string chain;
    std::string target;
    std::string protocol;
    uint16_t port;
    std::string source;
    std::string description;
};

class FirewallCheck {
public:
    FirewallCheck() = default;

    bool isFirewallActive() const;
    FirewallType getFirewallType() const;
    std::vector<FirewallRule> getRules() const;
    bool isPortOpen(uint16_t port) const;
    bool openPort(uint16_t port, const std::string& protocol = "tcp");
    bool closePort(uint16_t port, const std::string& protocol = "tcp");
    std::string getFirewallStatus() const;

private:
    std::string runCommand(const std::string& cmd) const;
    bool checkIptables() const;
    bool checkNftables() const;
    bool checkUfw() const;
    bool checkFirewalld() const;
};

} // namespace fml
