#include "dxvk_checker.h"

#include <sstream>
#include <cstdlib>
#include <fstream>

namespace fml {

bool DxvkChecker::isInstalled(const std::string& prefixPath) const {
    auto sys32 = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d11.dll";
    auto dxgi = std::filesystem::path(prefixPath) / "drive_c/windows/system32/dxgi.dll";
    return std::filesystem::exists(sys32) || std::filesystem::exists(dxgi);
}

std::string DxvkChecker::getDllVersion(const std::string& dllPath) const {
    if (!std::filesystem::exists(dllPath)) return "";

    std::string cmd = "strings \"" + dllPath + "\" 2>/dev/null | grep -i 'dxvk' | head -1";
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

std::string DxvkChecker::getVersion(const std::string& prefixPath) const {
    auto dllPath = std::filesystem::path(prefixPath) / "drive_c/windows/system32/d3d11.dll";
    std::string version = getDllVersion(dllPath.string());
    if (!version.empty()) return version;

    auto dxgiPath = std::filesystem::path(prefixPath) / "drive_c/windows/system32/dxgi.dll";
    return getDllVersion(dxgiPath.string());
}

bool DxvkChecker::downloadDxvk(const std::string& version, const std::string& targetDir) {
    std::string ver = (version == "latest") ? "" : version;
    std::string url;
    if (ver.empty()) {
        url = "https://github.com/doitsujin/dxvk/releases/latest/download/dxvk-" +
              ver + ".tar.gz";
        auto pipe = popen(
            "curl -sI 'https://github.com/doitsujin/dxvk/releases/latest' 2>/dev/null | grep -i location | tail -1",
            "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                std::string loc = buffer;
                size_t tagPos = loc.rfind('/');
                if (tagPos != std::string::npos) {
                    ver = loc.substr(tagPos + 1);
                    ver.erase(std::remove(ver.begin(), ver.end(), '\n'), ver.end());
                }
            }
            pclose(pipe);
        }
        if (ver.empty()) ver = "2.4";
    }

    url = "https://github.com/doitsujin/dxvk/releases/download/v" + ver +
          "/dxvk-" + ver + ".tar.gz";

    std::string cmd = "curl -L -o \"" + targetDir + "/dxvk.tar.gz\" '" + url + "' 2>/dev/null";
    int result = std::system(cmd.c_str());
    return result == 0 && std::filesystem::exists(targetDir + "/dxvk.tar.gz");
}

bool DxvkChecker::copyDlls(const std::string& dxvkDir, const std::string& system32,
                            const std::string& syswow64, bool is64bit) {
    auto dx64 = std::filesystem::path(dxvkDir) / "x64";
    auto dx32 = std::filesystem::path(dxvkDir) / "x32";

    std::error_code ec;
    if (std::filesystem::exists(dx64)) {
        for (auto& entry : std::filesystem::directory_iterator(dx64)) {
            if (entry.path().extension() == ".dll") {
                std::filesystem::copy_file(entry.path(),
                    std::filesystem::path(system32) / entry.path().filename(),
                    std::filesystem::copy_options::overwrite_existing, ec);
            }
        }
    }

    if (is64bit && std::filesystem::exists(dx32)) {
        for (auto& entry : std::filesystem::directory_iterator(dx32)) {
            if (entry.path().extension() == ".dll") {
                std::filesystem::copy_file(entry.path(),
                    std::filesystem::path(syswow64) / entry.path().filename(),
                    std::filesystem::copy_options::overwrite_existing, ec);
            }
        }
    }

    return !ec;
}

bool DxvkChecker::install(const std::string& prefixPath, const std::string& version) {
    auto tmpDir = std::filesystem::temp_directory_path() / "fml_dxvk";
    std::filesystem::create_directories(tmpDir);

    if (!downloadDxvk(version, tmpDir.string())) return false;

    std::string extractCmd = "tar -xzf \"" + tmpDir.string() + "/dxvk.tar.gz\" -C \"" +
                             tmpDir.string() + "\" 2>/dev/null";
    int result = std::system(extractCmd.c_str());
    if (result != 0) return false;

    std::string extractedDir;
    for (auto& entry : std::filesystem::directory_iterator(tmpDir)) {
        if (entry.is_directory() && entry.path().filename().string().find("dxvk") != std::string::npos) {
            extractedDir = entry.path().string();
            break;
        }
    }

    if (extractedDir.empty()) return false;

    auto sys32 = (std::filesystem::path(prefixPath) / "drive_c/windows/system32").string();
    auto syswow64 = (std::filesystem::path(prefixPath) / "drive_c/windows/syswow64").string();
    bool is64bit = std::filesystem::exists(syswow64);

    bool ok = copyDlls(extractedDir, sys32, syswow64, is64bit);

    std::filesystem::remove_all(tmpDir);
    return ok;
}

bool DxvkChecker::uninstall(const std::string& prefixPath) {
    auto sys32 = std::filesystem::path(prefixPath) / "drive_c/windows/system32";
    auto syswow64 = std::filesystem::path(prefixPath) / "drive_c/windows/syswow64";

    std::vector<std::string> dxvkDlls = {
        "d3d11.dll", "dxgi.dll", "d3d10core.dll", "d3d10_1.dll",
        "d3d10.dll", "d3d9.dll", "d3d8.dll", "ddraw.dll",
        "dinput8.dll", "dplaysvr.exe", "dplaysvr.dll"
    };

    std::error_code ec;
    for (auto& dll : dxvkDlls) {
        auto p1 = sys32 / dll;
        auto p2 = syswow64 / dll;
        if (std::filesystem::exists(p1)) std::filesystem::remove(p1, ec);
        if (std::filesystem::exists(p2)) std::filesystem::remove(p2, ec);
    }

    return true;
}

std::string DxvkChecker::getLatestVersion() const {
    auto pipe = popen(
        "curl -sI 'https://github.com/doitsujin/dxvk/releases/latest' 2>/dev/null | grep -i location",
        "r");
    if (!pipe) return "2.4";

    char buffer[256];
    std::string version = "2.4";
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

bool DxvkChecker::isSupported() const {
    return std::system("which vulkaninfo >/dev/null 2>&1") == 0;
}

} // namespace fml
