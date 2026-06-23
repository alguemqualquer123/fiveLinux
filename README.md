# FiveMLinuxSDK

**Advanced FiveM & GTA V compatibility layer for Linux**

A professional C++20 library/framework that provides comprehensive support for running, managing, diagnosing, and developing FiveM/GTA V on any Linux distribution, regardless of distro, kernel, or desktop environment.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20+-green.svg)](https://cmake.org/)

## Features

- **Multi-distro support**: Ubuntu, Debian, Arch, Fedora, Mint, Pop!_OS, SteamOS, and more
- **Wine/Proton management**: Create, repair, backup, and restore Wine prefixes
- **FiveM lifecycle**: Install, update, launch, and manage FiveM with a single tool
- **GPU detection**: Automatic NVIDIA/AMD/Intel GPU detection with Vulkan/DXVK/VKD3D status
- **Diagnostics**: Full system diagnostic reports with auto-repair suggestions
- **Network tools**: Latency testing, firewall checking, and port management
- **CLI interface**: Powerful command-line tool for all operations
- **Public API**: C++ API for integration into other applications

## Architecture

```
FiveMLinuxSDK/
├── core/           System detection, logging, config, environment
├── wine/           Prefix management, registry, DLL overrides, Proton
├── fivem/          Installer, updater, launcher, cache, server tools
├── gta/            Installation detector, file checker, repair system
├── graphics/       Vulkan, DXVK, VKD3D, GPU detection
├── network/        Latency tests, firewall, port management
├── cli/            Command-line interface
├── tests/          Unit and integration tests
└── docs/           Documentation
```

## Installation

### Prerequisites

- C++20 compiler (GCC 11+ or Clang 13+)
- CMake 3.20+
- Wine (for FiveM/GTA V)
- Vulkan drivers

### Build from source

```bash
git clone https://github.com/alguemqualquer123/fiveLinux.git
cd fiveLinux
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

### Build with Docker

```bash
docker-compose up --build
```

## Usage

### CLI Commands

```bash
# Show system status
fivem-linux status

# Run diagnostics
fivem-linux diagnose

# Auto-repair issues
fivem-linux repair

# Install FiveM
fivem-linux install

# Launch FiveM
fivem-linux launch

# Wine prefix management
fivem-linux wine create
fivem-linux wine info
fivem-linux wine repair

# Cache management
fivem-linux cache size
fivem-linux cache clear

# GPU information
fivem-linux gpu

# Network diagnostics
fivem-linux network ping
fivem-linux network ports
fivem-linux network firewall
```

### Public API (C++)

```cpp
#include <fivemlinux/fivemlinux.h>

int main() {
    auto& sdk = fml::FiveMLinuxSDK::instance();
    sdk.initialize();

    // Get system info
    auto sysInfo = sdk.getSystemInfo();
    std::cout << "Distro: " << sysInfo.distro_name << std::endl;

    // Get GPU info
    auto gpuInfo = sdk.getGpuInfo();
    std::cout << "GPU: " << gpuInfo.name << std::endl;

    // Run diagnostics
    sdk.diagnose();

    // Repair issues
    sdk.repair();

    sdk.shutdown();
    return 0;
}
```

## Building Tests

```bash
cd build
cmake -DFML_BUILD_TESTS=ON ..
make -j$(nproc)
ctest --output-on-failure
```

## Supported Platforms

| Distro | Status |
|--------|--------|
| Ubuntu 22.04+ | Supported |
| Ubuntu 24.04 | Supported |
| Debian 12+ | Supported |
| Arch Linux | Supported |
| Fedora 38+ | Supported |
| Linux Mint 21+ | Supported |
| Pop!_OS 22.04+ | Supported |
| SteamOS 3.x | Supported |

## Contributing

Contributions are welcome! Feel free to:

- **Fork** this repository
- **Open issues** for bugs or feature requests
- **Submit pull requests** with improvements
- **Share** with others who might find it useful

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [FiveM](https://fivem.net/) - The multiplayer modification for GTA V
- [Wine](https://www.winehq.org/) - Windows compatibility layer
- [DXVK](https://github.com/doitsujin/dxvk) - Vulkan-based D3D implementation
- [VKD3D-Proton](https://github.com/doitsujin/vkd3d-proton) - Direct3D 12 implementation

---

**Star this repo if you find it useful!** Your support helps keep the project alive and motivates further development.
