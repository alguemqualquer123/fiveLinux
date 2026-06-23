#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <filesystem>
#include <optional>
#include <variant>
#include <cstdint>
#include <chrono>
#include <sstream>

#ifdef FML_EXPORT
    #ifdef _WIN32
        #define FML_API __declspec(dllexport)
    #else
        #define FML_API __attribute__((visibility("default")))
    #endif
#else
    #ifdef _WIN32
        #define FML_API __declspec(dllimport)
    #else
        #define FML_API
    #endif
#endif

namespace fml {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

enum class DistroFamily : uint8_t {
    Unknown = 0,
    Debian,
    RedHat,
    Arch,
    SUSE,
    Gentoo,
    Alpine,
    Void,
    NixOS
};

enum class GpuVendor : uint8_t {
    Unknown = 0,
    NVIDIA,
    AMD,
    Intel,
    Qualcomm
};

enum class GraphicsAPI : uint8_t {
    Unknown = 0,
    Vulkan,
    OpenGL,
    DXVK,
    VKD3D,
    GalliumNine,
    WineD3D
};

enum class WineType : uint8_t {
    Unknown = 0,
    SystemWine,
    WineGE,
    Proton,
    ProtonExperimental,
    ProtonHotfix,
    Bottles,
    Lutris,
    Heroic
};

enum class FiveMStatus : uint8_t {
    Unknown = 0,
    NotInstalled,
    Installed,
    Running,
    Error,
    NeedsUpdate,
    MissingDependencies,
    CacheCorrupted
};

enum class InstallSource : uint8_t {
    Unknown = 0,
    Steam,
    EpicGames,
    RockstarLauncher,
    Manual
};

struct SystemInfo {
    std::string distro_id;
    std::string distro_name;
    std::string distro_version;
    DistroFamily family;
    std::string kernel_version;
    std::string arch;
    std::string hostname;
    uint32_t cpu_cores;
    uint64_t total_ram_mb;
};

struct GpuInfo {
    GpuVendor vendor;
    std::string name;
    std::string driver_version;
    uint64_t vram_mb;
    bool vulkan_supported;
    bool vulkan_available;
    std::string vulkan_version;
    std::vector<std::string> vulkan_extensions;
};

struct WineInfo {
    WineType type;
    std::string version;
    std::string path;
    std::string prefix_path;
    bool is64bit;
    std::string proton_version;
    std::map<std::string, std::string> registry_entries;
};

struct GraphicsState {
    bool vulkan_available;
    bool dxvk_available;
    bool vkd3d_available;
    bool mesa_available;
    bool opengl_available;
    std::string dxvk_version;
    std::string vkd3d_version;
    std::string mesa_version;
    std::string opengl_version;
    std::string driver_version;
    GpuVendor gpu_vendor;
    std::string gpu_name;
};

struct FiveMState {
    FiveMStatus status;
    std::string version;
    std::string install_path;
    std::string exe_path;
    std::string cache_path;
    std::string citizenfx_ini;
    bool has_gta_files;
    std::vector<std::string> missing_files;
    std::vector<std::string> corrupt_files;
    std::vector<std::string> warnings;
};

struct DiagnosticReport {
    bool success;
    std::string summary;
    struct Check {
        std::string name;
        bool passed;
        std::string message;
        std::string fix_suggestion;
    };
    std::vector<Check> checks;
    SystemInfo system;
    GpuInfo gpu;
    WineInfo wine;
    GraphicsState graphics;
    FiveMState fivem;
};

struct NetworkInfo {
    bool firewall_active;
    std::vector<uint16_t> open_ports;
    std::vector<uint16_t> required_ports;
    double latency_ms;
    bool connected;
};

using LogCallback = std::function<void(LogLevel, const std::string&, const std::string&)>;
using ProgressCallback = std::function<void(float, const std::string&)>;

}
