#include "vkd3d_checker.h"

#include <sstream>
#include <cstdlib>

namespace fml {

std::string Vkd3dChecker::getDllVersion(const std::string& dllPath) const {
    if (!std::filesystem::exists(dllPath)) return "";

    std::string cmd = "strings \"" + dllPath + "\" 2>/dev/null | grep -i 'vkd3d' | head -1";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buffer[256];
    std::string version;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        version = buffer;
        version.erase(std::remove(version.begin(), version.end(), '\n'), version.end());
    }
    pclose(pipe);
    return version;
}

bool Vkd3dChecker::isInstalled(const std::string& prefixPath) const {
    auto d3d12 = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d12.dll";
    auto core = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d12core.dll";
    return std::filesystem::exists(d3d12) || std::filesystem::exists(core);
}

std::string Vkd3dChecker::getVersion(const std::string& prefixPath) const {
    auto dllPath = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d12.dll";
    std::string version = getDllVersion(dllPath.string());
    if (!version.empty()) return version;

    auto corePath = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d12core.dll";
    return getDllVersion(corePath.string());
}

std::string Vkd3dChecker::getLatestVersion() const {
    auto pipe = popen(
        "curl -sI 'https://github.com/doitsujin/vkd3d-proton/releases/latest' 2>/dev/null | grep -i location",
        "r");
    if (!pipe) return "2.11";

    char buffer[256];
    std::string version = "2.11";
    if (fgets(buffer, sizeof(buffer), pipe)) {
        std::string loc = buffer;
        size_t tagPos = loc.rfind('/');
        if (tagPos != std::string::npos) {
            version = loc.substr(tagPos + 1);
            version.erase(std::remove(version.begin(), version.end(), '\n'), version.end());
        }
    }
    pclose(pipe);
    return version;
}

bool Vkd3dChecker::downloadVkd3d(const std::string& version, const std::string& targetDir) {
    std::string ver = (version == "latest") ? getLatestVersion() : version;
    std::string url = "https://github.com/doitsujin/vkd3d-proton/releases/download/v" +
                      ver + "/vkd3d-proton-" + ver + ".tar.xz";

    std::string cmd = "curl -L -o \"" + targetDir + "/vkd3d.tar.xz\" '" + url + "' 2>/dev/null";
    int result = std::system(cmd.c_str());
    return result == 0 && std::filesystem::exists(targetDir + "/vkd3d.tar.xz");
}

bool Vkd3dChecker::install(const std::string& prefixPath, const std::string& version) {
    auto tmpDir = std::filesystem::temp_directory_path() / "fml_vkd3d";
    std::filesystem::create_directories(tmpDir);

    if (!downloadVkd3d(version, tmpDir.string())) return false;

    std::string extractCmd = "tar -xJf \"" + tmpDir.string() + "/vkd3d.tar.xz\" -C \"" +
                             tmpDir.string() + "\" 2>/dev/null";
    int result = std::system(extractCmd.c_str());
    if (result != 0) return false;

    std::string extractedDir;
    for (auto& entry : std::filesystem::directory_iterator(tmpDir)) {
        if (entry.is_directory() && entry.path().filename().string().find("vkd3d") != std::string::npos) {
            extractedDir = entry.path().string();
            break;
        }
    }

    if (extractedDir.empty()) return false;

    auto x64Dir = std::filesystem::path(extractedDir) / "x64";
    auto x86Dir = std::filesystem::path(extractedDir) / "x86";
    auto sys32 = std::filesystem::path(prefixPath) / "drive_c/windows/system32";
    auto syswow64 = std::filesystem::path(prefixPath) / "drive_c/windows/syswow64";
    bool is64bit = std::filesystem::exists(syswow64);

    std::error_code ec;
    if (std::filesystem::exists(x64Dir)) {
        for (auto& entry : std::filesystem::directory_iterator(x64Dir)) {
            if (entry.path().extension() == ".dll") {
                std::filesystem::copy_file(entry.path(),
                    sys32 / entry.path().filename(),
                    std::filesystem::copy_options::overwrite_existing, ec);
            }
        }
    }

    if (is64bit && std::filesystem::exists(x86Dir)) {
        for (auto& entry : std::filesystem::directory_iterator(x86Dir)) {
            if (entry.path().extension() == ".dll") {
                std::filesystem::copy_file(entry.path(),
                    syswow64 / entry.path().filename(),
                    std::filesystem::copy_options::overwrite_existing, ec);
            }
        }
    }

    std::filesystem::remove_all(tmpDir);
    return !ec;
}

bool Vkd3dChecker::uninstall(const std::string& prefixPath) {
    auto sys32 = std::filesystem::path(prefixPath) / "drive_c/windows/system32";
    auto syswow64 = std::filesystem::path(prefixPath) / "drive_c/windows/syswow64";

    std::vector<std::string> vkd3dDlls = {"d3d12.dll", "d3d12core.dll", "d3d12sdklayers.dll"};

    std::error_code ec;
    for (auto& dll : vkd3dDlls) {
        auto p1 = sys32 / dll;
        auto p2 = syswow64 / dll;
        if (std::filesystem::exists(p1)) std::filesystem::remove(p1, ec);
        if (std::filesystem::exists(p2)) std::filesystem::remove(p2, ec);
    }

    return true;
}

} // namespace fml
