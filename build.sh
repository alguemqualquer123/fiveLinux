#!/bin/bash
# FiveMLinuxSDK - Easy Build Script
# Compiles the project with a single command

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()  { echo -e "${BLUE}[INFO]${NC} $1"; }
ok()    { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"
JOBS=$(nproc 2>/dev/null || echo 4)

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}   FiveMLinuxSDK - Build Script             ${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""

info "Build type: $BUILD_TYPE"
info "Parallel jobs: $JOBS"
echo ""

# Check cmake
if ! command -v cmake &>/dev/null; then
    echo -e "${RED}CMake not found! Run ./install-deps.sh first${NC}"
    exit 1
fi

# Check compiler
if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
    echo -e "${RED}C++ compiler not found! Run ./install-deps.sh first${NC}"
    exit 1
fi

# Create build dir
info "Creating build directory..."
mkdir -p "$BUILD_DIR"

# Configure
info "Configuring with CMake..."
cd "$BUILD_DIR"
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DFML_BUILD_CLI=ON \
    -DFML_BUILD_TESTS=ON \
    -DFML_BUILD_GUI=OFF

# Build
info "Building..."
cmake --build . --config "$BUILD_TYPE" -j "$JOBS"

echo ""
ok "Build complete!"
echo ""
echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}   Build artifacts:                         ${NC}"
echo -e "${GREEN}   - CLI:     $BUILD_DIR/src/cli/fivem-linux${NC}"
echo -e "${GREEN}   - Library: $BUILD_DIR/libfivemlinux.so   ${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""

# Ask to install
read -p "Install to system? (y/N): " INSTALL
if [[ "$INSTALL" =~ ^[Yy]$ ]]; then
    info "Installing..."
    sudo cmake --install .
    ok "Installed! Run: fivem-linux --help"
else
    info "Skipped installation."
    info "To install later: cd $BUILD_DIR && sudo cmake --install ."
fi
