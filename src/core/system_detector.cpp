#include "system_detector.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

#ifdef __linux__
#include <sys/utsname.h>
#include <unistd.h>
#endif

namespace fml {

SystemDetector::SystemDetector() {
    info_.family = DistroFamily::Unknown;
}

void SystemDetector::detect() {
    readOsRelease();
    readUname();
    readCpuInfo();
    readMemInfo();
    detectDesktop();
    detectSudo();
}

void SystemDetector::readOsRelease() {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        if (key == "ID") {
            info_.distro_id = value;
            if (value == "ubuntu" || value == "debian" || value == "linuxmint" ||
                value == "pop" || value == "elementary" || value == "kali") {
                info_.family = DistroFamily::Debian;
            } else if (value == "fedora" || value == "rhel" || value == "centos" ||
                       value == "rocky" || value == "alma" || value == "ol") {
                info_.family = DistroFamily::RedHat;
            } else if (value == "arch" || value == "manjaro" || value == "endeavouros") {
                info_.family = DistroFamily::Arch;
            } else if (value == "opensuse" || value == "sles" || value == "suse") {
                info_.family = DistroFamily::SUSE;
            } else if (value == "gentoo") {
                info_.family = DistroFamily::Gentoo;
            } else if (value == "alpine") {
                info_.family = DistroFamily::Alpine;
            } else if (value == "void") {
                info_.family = DistroFamily::Void;
            } else if (value == "nixos") {
                info_.family = DistroFamily::NixOS;
            }
        } else if (key == "NAME") {
            info_.distro_name = value;
        } else if (key == "VERSION_ID") {
            info_.distro_version = value;
        }
    }
}

void SystemDetector::readUname() {
#ifdef __linux__
    struct utsname uts;
    if (uname(&uts) == 0) {
        info_.kernel_version = uts.release;
        info_.arch = uts.machine;
        info_.hostname = uts.nodename;
    }
#endif
}

void SystemDetector::readCpuInfo() {
#ifdef __linux__
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return;

    uint32_t coreCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 9) == "processor") {
            coreCount++;
        }
    }
    info_.cpu_cores = coreCount > 0 ? coreCount : 1;
#endif
}

void SystemDetector::readMemInfo() {
#ifdef __linux__
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("MemTotal") == 0) {
            std::istringstream iss(line);
            std::string label, unit;
            uint64_t kb;
            iss >> label >> kb >> unit;
            info_.total_ram_mb = kb / 1024;
            break;
        }
    }
#endif
}

void SystemDetector::detectDesktop() {
    const char* xdgDesktop = std::getenv("XDG_CURRENT_DESKTOP");
    if (xdgDesktop) {
        std::string desktop(xdgDesktop);
        for (auto& c : desktop) c = static_cast<char>(std::toupper(c));
        wayland_ = desktop.find("WAYLAND") != std::string::npos;
    }

    if (!wayland_) {
        const char* waylandDisplay = std::getenv("WAYLAND_DISPLAY");
        wayland_ = waylandDisplay != nullptr && std::string(waylandDisplay).length() > 0;
    }
}

void SystemDetector::detectSudo() {
#ifdef __linux__
    sudo_ = (getuid() == 0);
    if (!sudo_) {
        sudo_ = (std::system("sudo -n true") == 0);
    }
#endif
}

const SystemInfo& SystemDetector::getSystemInfo() const {
    return info_;
}

uint64_t SystemDetector::getUptime() const {
#ifdef __linux__
    std::ifstream file("/proc/uptime");
    if (!file.is_open()) return 0;

    double uptimeSeconds;
    file >> uptimeSeconds;
    return static_cast<uint64_t>(uptimeSeconds);
#else
    return 0;
#endif
}

bool SystemDetector::isWayland() const {
    return wayland_;
}

bool SystemDetector::hasSudo() const {
    return sudo_;
}

} // namespace fml