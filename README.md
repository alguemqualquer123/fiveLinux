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

## Quick Start (3 commands)

```bash
git clone https://github.com/alguemqualquer123/fiveLinux.git
cd fiveLinux
./install-deps.sh && ./build.sh
```

That's it! The scripts automatically detect your distro and install everything needed.

## Installation

### One-Line Install

```bash
git clone https://github.com/alguemqualquer123/fiveLinux.git && cd fiveLinux && ./install-deps.sh && ./build.sh
```

### Step by Step

**1. Clone the repository:**
```bash
git clone https://github.com/alguemqualquer123/fiveLinux.git
cd fiveLinux
```

**2. Install dependencies (auto-detects your distro):**
```bash
./install-deps.sh
```

**3. Build the project:**
```bash
./build.sh
```

**4. Install to system (optional):**
```bash
sudo cmake --install build/
```

**5. Run:**
```bash
fivem-linux --help
```

### Build Options

```bash
# Debug build
./build.sh Debug

# Release build (default)
./build.sh Release

# Build and run immediately
./run.sh status
```

### Build with Docker

```bash
docker-compose up --build
```

## CLI Commands

```bash
# Show system status
fivem-linux status

# Run full diagnostics
fivem-linux diagnose

# Auto-repair common issues
fivem-linux repair

# Install FiveM
fivem-linux install

# Launch FiveM
fivem-linux launch

# Wine prefix management
fivem-linux wine create    # Create new prefix
fivem-linux wine info      # Show prefix info
fivem-linux wine repair    # Repair prefix
fivem-linux wine proton    # Show Proton versions

# Rockstar Games Launcher
fivem-linux rockstar launch     # Launch Rockstar Launcher
fivem-linux rockstar status     # Show launcher status
fivem-linux rockstar games      # List installed Rockstar games
fivem-linux rockstar setup      # Setup Wine prefix for Rockstar
fivem-linux rockstar install-deps  # Install VC++ Redistributables
fivem-linux rockstar login      # Login to Rockstar account
fivem-linux rockstar steam-login # Login via Steam

# Cache management
fivem-linux cache size     # Show cache size
fivem-linux cache clear    # Clear cache
fivem-linux cache list     # List cache contents

# GPU information
fivem-linux gpu            # Show GPU details

# Network diagnostics
fivem-linux network ping       # Test FiveM servers
fivem-linux network ports      # Check required ports
fivem-linux network firewall   # Firewall status
```

## Public API (C++)

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

### Compile with the library

```bash
g++ -std=c++20 myapp.cpp -lfivemlinux -o myapp
```

## Architecture

```
FiveMLinuxSDK/
├── core/           System detection, logging, config, environment
├── wine/           Prefix management, registry, DLL overrides, Proton
├── fivem/          Installer, updater, launcher, cache, server tools
├── gta/            Installation detector, file checker, repair system
├── graphics/       Vulkan, DXVK, VKD3D, GPU detection
├── network/        Latency tests, firewall, port management
├── rockstar/       Rockstar Games Launcher, Social Club, account management
├── cli/            Command-line interface (animated)
├── tests/          Unit and integration tests
└── docs/           Documentation
```

## Required Programs

| Program | Purpose | Install |
|---------|---------|---------|
| GCC/G++ | C++ compiler | `sudo apt install build-essential` |
| CMake | Build system | `sudo apt install cmake` |
| Wine | Windows compatibility | https://www.winehq.org/ |
| Vulkan | Graphics API | `sudo apt install vulkan-tools` |
| curl | Download tool | `sudo apt install curl` |
| 7zip | Archive extraction | `sudo apt install p7zip-full` |

## Useful Links

### Wine & Proton
| Tool | Link | Description |
|------|------|-------------|
| Wine | https://www.winehq.org/ | Windows compatibility layer |
| Wine-GE | https://github.com/GloriousEggroll/wine-ge-custom | Custom Wine build for gaming |
| Proton | https://github.com/ValveSoftware/Proton | Valve's Wine fork for Steam |
| Proton-GE | https://github.com/GloriousEggroll/proton-ge-custom | Community Proton build |
| ProtonDB | https://www.protondb.com/ | Compatibility reports |

### DXVK & VKD3D
| Tool | Link | Description |
|------|------|-------------|
| DXVK | https://github.com/doitsujin/dxvk | Vulkan-based D3D9/10/11 |
| VKD3D-Proton | https://github.com/doitsujin/vkd3d-proton | Vulkan-based D3D12 |

### FiveM & GTA V
| Tool | Link | Description |
|------|------|-------------|
| FiveM | https://fivem.net/ | Multiplayer modification |
| FiveM Docs | https://docs.fivem.net/ | Documentation |
| FiveM Artifacts | https://runtime.fivem.net/artifacts/ | Build downloads |
| GTA V (Steam) | https://store.steampowered.com/app/271590/ | Buy GTA V |
| GTA V (Rockstar) | https://www.rockstargames.com/games/V | Buy from Rockstar |

### Rockstar Games
| Tool | Link | Description |
|------|------|-------------|
| Rockstar Launcher | https://www.rockstargames.com/downloads | Download launcher |
| Social Club | https://socialclub.rockstargames.com/ | Account management |
| Rockstar Support | https://support.rockstargames.com/ | Help & support |
| RDR2 (Steam) | https://store.steampowered.com/app/1174180/ | Buy RDR2 |

### GPU Drivers
| Vendor | Link |
|--------|------|
| NVIDIA | https://www.nvidia.com/drivers |
| AMD | https://www.amd.com/en/support |
| Intel | https://www.intel.com/content/www/us/en/download-center/home.html |

### Alternative Launchers
| Launcher | Link | Description |
|----------|------|-------------|
| Bottles | https://usebottles.com/ | Flatpak Wine manager |
| Lutris | https://lutris.net/ | Gaming platform |
| Heroic | https://heroicgameslauncher.com/ | Epic/GOG/Prime launcher |
| Steam | https://store.steampowered.com/ | Valve's platform |

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
| Manjaro | Supported |
| openSUSE | Supported |

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

**Want to contribute?** Fork the repo, make your changes, and submit a pull request. All contributions are welcome!
