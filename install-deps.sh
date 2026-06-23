#!/bin/bash
# FiveMLinuxSDK - Dependency Installer
# Detects your distro and installs everything needed to build

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()  { echo -e "${BLUE}[INFO]${NC} $1"; }
ok()    { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
err()   { echo -e "${RED}[ERROR]${NC} $1"; }

detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO_ID="${ID}"
        DISTRO_LIKE="${ID_LIKE:-$ID}"
    else
        DISTRO_ID="unknown"
        DISTRO_LIKE="unknown"
    fi
    info "Detected: $DISTRO_ID ($DISTRO_LIKE)"
}

install_deps_debian() {
    info "Installing dependencies for Debian/Ubuntu..."
    sudo apt update
    sudo apt install -y \
        build-essential \
        cmake \
        g++ \
        git \
        curl \
        wget \
        7zip \
        p7zip-full \
        wine \
        winetricks \
        vulkan-tools \
        mesa-vulkan-drivers \
        libvulkan-dev \
        pciutils \
        nvidia-smi || true
    ok "Debian/Ubuntu dependencies installed"
}

install_deps_fedora() {
    info "Installing dependencies for Fedora/RHEL..."
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        make \
        git \
        curl \
        wget \
        p7zip-plugins \
        wine \
        winetricks \
        vulkan-tools \
        mesa-vulkan-drivers \
        vulkan-loader-devel \
        pciutils || true
    ok "Fedora dependencies installed"
}

install_deps_arch() {
    info "Installing dependencies for Arch Linux..."
    sudo pacman -S --needed --noconfirm \
        base-devel \
        cmake \
        git \
        curl \
        wget \
        p7zip \
        wine \
        winetricks \
        vulkan-tools \
        mesa \
        lib32-mesa \
        pciutils || true
    ok "Arch dependencies installed"
}

install_deps_opensuse() {
    info "Installing dependencies for openSUSE..."
    sudo zypper install -y \
        gcc-c++ \
        cmake \
        make \
        git \
        curl \
        wget \
        p7zip \
        wine \
        vulkan-tools \
        Mesa-vulkan-drivers \
        pciutils || true
    ok "openSUSE dependencies installed"
}

print_links() {
    echo ""
    echo -e "${BLUE}=== Useful Links ===${NC}"
    echo ""
    echo "Wine:"
    echo "  - WineHQ:        https://www.winehq.org/"
    echo "  - Wine-GE:       https://github.com/GloriousEggroll/proton-ge-custom"
    echo "  - Proton-GE:     https://github.com/GloriousEggroll/wine-ge-custom"
    echo ""
    echo "Proton:"
    echo "  - Proton:        https://github.com/ValveSoftware/Proton"
    echo "  - Proton-GE:     https://github.com/GloriousEggroll/proton-ge-custom"
    echo "  - ProtonDB:      https://www.protondb.com/"
    echo ""
    echo "DXVK / VKD3D:"
    echo "  - DXVK:          https://github.com/doitsujin/dxvk"
    echo "  - VKD3D-Proton:  https://github.com/doitsujin/vkd3d-proton"
    echo ""
    echo "FiveM:"
    echo "  - FiveM:         https://fivem.net/"
    echo "  - FiveM Linux:   https://docs.fivem.net/"
    echo "  - FiveM Artifacts: https://runtime.fivem.net/artifacts/"
    echo ""
    echo "GTA V:"
    echo "  - GTA V Steam:   https://store.steampowered.com/app/271590/"
    echo "  - Rockstar:      https://www.rockstargames.com/"
    echo ""
    echo "GPU Drivers:"
    echo "  - NVIDIA:        https://www.nvidia.com/drivers"
    echo "  - AMD:           https://www.amd.com/en/support"
    echo "  - Intel:         https://www.intel.com/content/www/us/en/download-center/home.html"
    echo ""
    echo "Tools:"
    echo "  - Bottles:       https://usebottles.com/"
    echo "  - Lutris:        https://lutris.net/"
    echo "  - Heroic:        https://heroicgameslauncher.com/"
    echo ""
}

main() {
    echo -e "${BLUE}============================================${NC}"
    echo -e "${BLUE}   FiveMLinuxSDK - Dependency Installer     ${NC}"
    echo -e "${BLUE}============================================${NC}"
    echo ""

    detect_distro

    case "$DISTRO_ID" in
        ubuntu|debian|linuxmint|pop|zorin|elementary|kali|deepin)
            install_deps_debian
            ;;
        fedora|rhel|centos|rocky|alma)
            install_deps_fedora
            ;;
        arch|manjaro|endeavouros|garuda|artix)
            install_deps_arch
            ;;
        opensuse*|suse|sles)
            install_deps_opensuse
            ;;
        *)
            warn "Unknown distro: $DISTRO_ID"
            warn "Attempting Debian-based install..."
            install_deps_debian
            ;;
    esac

    echo ""
    ok "All dependencies installed!"
    echo ""
    info "Now run: ./build.sh"
    echo ""

    print_links
}

main "$@"
