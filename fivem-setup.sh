#!/bin/bash
# FiveMLinuxSDK - Auto-Install Script
# Downloads and sets up FiveM with one click
# Place this next to FiveM.exe to auto-detect and install

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FML_DIR="$HOME/.fivem-linux"
PREFIX_DIR="$FML_DIR/prefix"
FIVEM_DIR="$FML_DIR/FiveM"
LOG_FILE="$FML_DIR/install.log"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

log() { echo "[$(date '+%H:%M:%S')] $1" >> "$LOG_FILE"; echo -e "$1"; }

check_deps() {
    log "${CYAN}Checking dependencies...${NC}"
    local missing=()

    command -v wine &>/dev/null || missing+=("wine")
    command -v curl &>/dev/null || missing+=("curl")
    command -v 7z &>/dev/null && command -v p7zip &>/dev/null || missing+=("p7zip-full")

    if [ ${#missing[@]} -gt 0 ]; then
        log "${YELLOW}Missing: ${missing[*]}${NC}"
        log "${CYAN}Installing dependencies...${NC}"
        if command -v apt &>/dev/null; then
            sudo apt update && sudo apt install -y wine curl p7zip-full
        elif command -v dnf &>/dev/null; then
            sudo dnf install -y wine curl p7zip
        elif command -v pacman &>/dev/null; then
            sudo pacman -S --needed wine curl p7zip
        else
            log "${RED}Cannot auto-install. Install: wine, curl, p7zip${NC}"
            exit 1
        fi
    fi
    log "${GREEN}Dependencies OK${NC}"
}

create_prefix() {
    log "${CYAN}Creating Wine prefix...${NC}"
    if [ ! -d "$PREFIX_DIR" ]; then
        mkdir -p "$PREFIX_DIR"
        WINEPREFIX="$PREFIX_DIR" wineboot -u 2>/dev/null
        log "${GREEN}Prefix created at $PREFIX_DIR${NC}"
    else
        log "${GREEN}Prefix already exists${NC}"
    fi
}

setup_dxvk() {
    log "${CYAN}Setting up DXVK...${NC}"
    local dxvk_dir="$FML_DIR/dxvk"
    local sys32="$PREFIX_DIR/drive_c/windows/system32"

    if [ -f "$sys32/d3d11.dll" ]; then
        log "${GREEN}DXVK already installed${NC}"
        return
    fi

    mkdir -p "$dxvk_dir"
    local latest="2.4"
    local url="https://github.com/doitsujin/dxvk/releases/download/v${latest}/dxvk-${latest}.tar.gz"

    log "${CYAN}Downloading DXVK ${latest}...${NC}"
    curl -sL -o "$dxvk_dir/dxvk.tar.gz" "$url"
    tar -xzf "$dxvk_dir/dxvk.tar.gz" -C "$dxvk_dir"

    local extracted="$dxvk_dir/dxvk-${latest}"
    if [ -d "$extracted/x64" ]; then
        cp -f "$extracted/x64/"*.dll "$sys32/" 2>/dev/null
        local syswow64="$PREFIX_DIR/drive_c/windows/syswow64"
        if [ -d "$syswow64" ] && [ -d "$extracted/x32" ]; then
            cp -f "$extracted/x32/"*.dll "$syswow64/" 2>/dev/null
        fi
        log "${GREEN}DXVK installed${NC}"
    fi

    rm -rf "$dxvk_dir"
}

install_fivem() {
    log "${CYAN}Downloading FiveM...${NC}"
    mkdir -p "$FIVEM_DIR"

    local build_url="https://runtime.fivem.net/artifacts/fivem/build_proton_linux/master/"
    local page=$(curl -sL "$build_url" 2>/dev/null)

    local file_url=$(echo "$page" | grep -oP '"file":"[^"]*"' | head -1 | cut -d'"' -f4)
    if [ -z "$file_url" ]; then
        file_url="https://runtime.fivem.net/artifacts/fivem/build_proton_linux/master/fivem.tar.gz"
    fi

    if [[ "$file_url" != http* ]]; then
        file_url="https://runtime.fivem.net${file_url}"
    fi

    log "${CYAN}Downloading from: $file_url${NC}"
    curl -sL -o "$FIVEM_DIR/fivem.tar.gz" "$file_url"

    if [ -f "$FIVEM_DIR/fivem.tar.gz" ]; then
        tar -xzf "$FIVEM_DIR/fivem.tar.gz" -C "$FIVEM_DIR" 2>/dev/null
        rm -f "$FIVEM_DIR/fivem.tar.gz"
        log "${GREEN}FiveM installed to $FIVEM_DIR${NC}"
    else
        log "${RED}Download failed. Check network.${NC}"
        return 1
    fi

    mkdir -p "$FIVEM_DIR/cache/game"
    mkdir -p "$FIVEM_DIR/cache/priv"
    mkdir -p "$FIVEM_DIR/cache/files"
    mkdir -p "$FIVEM_DIR/cache/server-cache-priv"
}

launch_fivem() {
    log "${CYAN}Launching FiveM...${NC}"
    local exe="$FIVEM_DIR/FiveM.exe"

    if [ ! -f "$exe" ]; then
        log "${RED}FiveM.exe not found at $exe${NC}"
        log "${YELLOW}Install FiveM first or place FiveM.exe in: $FIVEM_DIR${NC}"
        return 1
    fi

    export WINEPREFIX="$PREFIX_DIR"
    export WINEDEBUG="-all"
    export DXVK_HUD="fps,version,compiler"
    export WINE_FULLSCREEN_FSR="1"

    cd "$FIVEM_DIR"
    wine "$exe" &

    log "${GREEN}FiveM launched!${NC}"
    log "${CYAN}PID: $!${NC}"
}

show_status() {
    echo ""
    echo -e "${BOLD}=== FiveMLinux Status ===${NC}"
    echo ""

    echo -e "${CYAN}Prefix:${NC} $([ -d "$PREFIX_DIR" ] && echo 'OK' || echo 'Not created')"
    echo -e "${CYAN}DXVK:${NC} $([ -f "$PREFIX_DIR/drive_c/windows/system32/d3d11.dll" ] && echo 'Installed' || echo 'Not installed')"
    echo -e "${CYAN}FiveM:${NC} $([ -f "$FIVEM_DIR/FiveM.exe" ] && echo 'Installed' || echo 'Not installed')"

    if [ -f "$FIVEM_DIR/FiveM.exe" ]; then
        echo -e "${CYAN}Path:${NC} $FIVEM_DIR"
    fi

    echo ""
}

usage() {
    echo ""
    echo -e "${BOLD}FiveMLinuxSDK - Auto Setup${NC}"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  install    Full setup (prefix + DXVK + FiveM)"
    echo "  launch     Launch FiveM"
    echo "  status     Show installation status"
    echo "  prefix     Create Wine prefix only"
    echo "  dxvk       Install DXVK only"
    echo "  fivem      Download FiveM only"
    echo ""
    echo "No args: interactive setup"
    echo ""
}

main() {
    mkdir -p "$FML_DIR"
    > "$LOG_FILE"

    case "${1:-}" in
        install)
            check_deps
            create_prefix
            setup_dxvk
            install_fivem
            show_status
            ;;
        launch)
            launch_fivem
            ;;
        status)
            show_status
            ;;
        prefix)
            check_deps
            create_prefix
            ;;
        dxvk)
            setup_dxvk
            ;;
        fivem)
            install_fivem
            ;;
        --help|-h|"")
            usage
            if [ -z "$1" ]; then
                echo -e "${CYAN}Running interactive setup...${NC}"
                echo ""
                check_deps
                create_prefix
                setup_dxvk
                install_fivem
                show_status
                echo -e "${GREEN}Setup complete! Run: $0 launch${NC}"
            fi
            ;;
        *)
            echo "Unknown command: $1"
            usage
            ;;
    esac
}

main "$@"
