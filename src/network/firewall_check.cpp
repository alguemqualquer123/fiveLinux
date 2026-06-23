#include "firewall_check.h"

#include <sstream>
#include <cstdlib>

namespace fml {

std::string FirewallCheck::runCommand(const std::string& cmd) const {
    auto pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) return "";

    std::ostringstream result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result << buffer;
    }
    pclose(pipe);
    return result.str();
}

bool FirewallCheck::checkIptables() const {
    std::string output = runCommand("iptables -L -n 2>/dev/null");
    return !output.empty() && output.find("error") == std::string::npos;
}

bool FirewallCheck::checkNftables() const {
    std::string output = runCommand("nft list ruleset 2>/dev/null");
    return !output.empty() && output.find("error") == std::string::npos;
}

bool FirewallCheck::checkUfw() const {
    std::string output = runCommand("ufw status 2>/dev/null");
    return output.find("active") != std::string::npos;
}

bool FirewallCheck::checkFirewalld() const {
    std::string output = runCommand("firewall-cmd --state 2>/dev/null");
    return output.find("running") != std::string::npos;
}

bool FirewallCheck::isFirewallActive() const {
    return getFirewallType() != FirewallType::None;
}

FirewallType FirewallCheck::getFirewallType() const {
    if (checkUfw()) return FirewallType::Ufw;
    if (checkFirewalld()) return FirewallType::Firewalld;
    if (checkNftables()) return FirewallType::Nftables;
    if (checkIptables()) return FirewallType::Iptables;
    return FirewallType::None;
}

std::vector<FirewallRule> FirewallCheck::getRules() const {
    std::vector<FirewallRule> rules;
    FirewallType type = getFirewallType();

    if (type == FirewallType::Ufw) {
        std::string output = runCommand("ufw status numbered 2>/dev/null");
        std::istringstream stream(output);
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("[") == 0 && line.find("ALLOW") != std::string::npos) {
                FirewallRule rule{};
                rule.target = "ALLOW";
                size_t portStart = line.find(" ");
                size_t portEnd = line.find(" ", portStart + 1);
                if (portStart != std::string::npos) {
                    std::string portStr = line.substr(portStart + 1, portEnd - portStart - 1);
                    size_t slash = portStr.find('/');
                    if (slash != std::string::npos) {
                        try {
                            rule.port = static_cast<uint16_t>(std::stoi(portStr.substr(0, slash)));
                        } catch (...) {}
                        rule.protocol = portStr.substr(slash + 1);
                    }
                }
                rules.push_back(rule);
            }
        }
    } else if (type == FirewallType::Iptables) {
        std::string output = runCommand("iptables -L -n --line-numbers 2>/dev/null");
        std::istringstream stream(output);
        std::string line;

        while (std::getline(stream, line)) {
            if (line.find("ACCEPT") != std::string::npos && line.find("dpt:") != std::string::npos) {
                FirewallRule rule{};
                rule.target = "ACCEPT";
                size_t dptPos = line.find("dpt:");
                if (dptPos != std::string::npos) {
                    std::string portStr = line.substr(dptPos + 4);
                    size_t end = portStr.find(' ');
                    if (end != std::string::npos) portStr = portStr.substr(0, end);
                    try {
                        rule.port = static_cast<uint16_t>(std::stoi(portStr));
                    } catch (...) {}
                }
                rules.push_back(rule);
            }
        }
    }

    return rules;
}

bool FirewallCheck::isPortOpen(uint16_t port) const {
    std::string cmd = "ss -tlnp 2>/dev/null | grep ':" + std::to_string(port) + " '";
    return std::system(cmd.c_str()) == 0;
}

bool FirewallCheck::openPort(uint16_t port, const std::string& protocol) {
    FirewallType type = getFirewallType();
    std::string portStr = std::to_string(port);

    switch (type) {
        case FirewallType::Ufw:
            return std::system(("ufw allow " + portStr + "/" + protocol + " 2>/dev/null").c_str()) == 0;
        case FirewallType::Iptables:
            return std::system(("iptables -A INPUT -p " + protocol + " --dport " +
                               portStr + " -j ACCEPT 2>/dev/null").c_str()) == 0;
        case FirewallType::Firewalld:
            return std::system(("firewall-cmd --add-port=" + portStr + "/" + protocol +
                               " --permanent 2>/dev/null && firewall-cmd --reload 2>/dev/null").c_str()) == 0;
        default:
            return true;
    }
}

bool FirewallCheck::closePort(uint16_t port, const std::string& protocol) {
    FirewallType type = getFirewallType();
    std::string portStr = std::to_string(port);

    switch (type) {
        case FirewallType::Ufw:
            return std::system(("ufw delete allow " + portStr + "/" + protocol + " 2>/dev/null").c_str()) == 0;
        case FirewallType::Iptables:
            return std::system(("iptables -D INPUT -p " + protocol + " --dport " +
                               portStr + " -j ACCEPT 2>/dev/null").c_str()) == 0;
        case FirewallType::Firewalld:
            return std::system(("firewall-cmd --remove-port=" + portStr + "/" + protocol +
                               " --permanent 2>/dev/null && firewall-cmd --reload 2>/dev/null").c_str()) == 0;
        default:
            return true;
    }
}

std::string FirewallCheck::getFirewallStatus() const {
    FirewallType type = getFirewallType();
    switch (type) {
        case FirewallType::Ufw: return "UFW (active)";
        case FirewallType::Nftables: return "nftables (active)";
        case FirewallType::Iptables: return "iptables (active)";
        case FirewallType::Firewalld: return "firewalld (active)";
        case FirewallType::None: return "No firewall detected";
        default: return "Unknown firewall";
    }
}

} // namespace fml
