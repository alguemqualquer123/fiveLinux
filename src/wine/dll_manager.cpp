#include "dll_manager.h"

#include <fstream>
#include <sstream>

namespace fml {

DllManager::DllManager(const std::string& prefixPath)
    : prefixPath_(prefixPath) {
    parseDllOverrides();
}

std::string DllManager::getSystem32Path() const {
    return prefixPath_ + "/drive_c/windows/system32";
}

std::string DllManager::getSysWOW64Path() const {
    return prefixPath_ + "/drive_c/windows/syswow64";
}

bool DllManager::parseDllOverrides() {
    overrides_.clear();

    std::string userRegPath = prefixPath_ + "/user.reg";
    if (!std::filesystem::exists(userRegPath)) return false;

    std::ifstream file(userRegPath);
    if (!file.is_open()) return false;

    std::string line;
    std::string currentKey;
    bool inDllOverrides = false;

    while (std::getline(file, line)) {
        if (line.find("[Software\\\\Wine\\\\DllOverrides]") != std::string::npos) {
            inDllOverrides = true;
            continue;
        }

        if (inDllOverrides && line[0] == '[') {
            break;
        }

        if (inDllOverrides && !line.empty() && line[0] != ';') {
            auto eq = line.find('=');
            if (eq != std::string::npos) {
                std::string dllName = line.substr(1, eq - 2);
                std::string overrideStr = line.substr(eq + 1);
                overrideStr.erase(std::remove(overrideStr.begin(), overrideStr.end(), '"'), overrideStr.end());
                overrideStr.erase(std::remove(overrideStr.begin(), overrideStr.end(), '\r'), overrideStr.end());

                DllOverrideType type = DllOverrideType::Native;
                if (overrideStr == "builtin") type = DllOverrideType::Builtin;
                else if (overrideStr == "disabled") type = DllOverrideType::Disabled;
                else if (overrideStr == "native,builtin") type = DllOverrideType::Native;
                else if (overrideStr == "builtin,native") type = DllOverrideType::Builtin;

                overrides_[dllName] = type;
            }
        }
    }

    return true;
}

std::map<std::string, DllOverrideType> DllManager::getDllOverrides() {
    return overrides_;
}

bool DllManager::setDllOverride(const std::string& dllName, DllOverrideType type) {
    overrides_[dllName] = type;
    return writeDllOverrides();
}

bool DllManager::removeDllOverride(const std::string& dllName) {
    auto it = overrides_.find(dllName);
    if (it == overrides_.end()) return false;
    overrides_.erase(it);
    return writeDllOverrides();
}

bool DllManager::writeDllOverrides() {
    std::string userRegPath = prefixPath_ + "/user.reg";
    std::ifstream inFile(userRegPath);
    std::string content;

    if (inFile.is_open()) {
        std::ostringstream ss;
        ss << inFile.rdbuf();
        content = ss.str();
        inFile.close();
    }

    size_t sectionStart = content.find("[Software\\\\Wine\\\\DllOverrides]");
    if (sectionStart != std::string::npos) {
        size_t sectionEnd = content.find('\n', sectionStart);
        while (sectionEnd != std::string::npos && sectionEnd + 1 < content.size() && content[sectionEnd + 1] == '\\') {
            sectionEnd = content.find('\n', sectionEnd + 1);
        }
        content.erase(sectionStart, sectionEnd - sectionStart);
    }

    std::string section = "[Software\\\\Wine\\\\DllOverrides]\n";
    for (auto& [name, type] : overrides_) {
        std::string typeStr;
        switch (type) {
            case DllOverrideType::Native: typeStr = "native,builtin"; break;
            case DllOverrideType::Builtin: typeStr = "builtin,native"; break;
            case DllOverrideType::Disabled: typeStr = "disabled"; break;
        }
        section += "\"" + name + "\"=\"" + typeStr + "\"\n";
    }
    section += "\n";

    if (sectionStart != std::string::npos) {
        content.insert(sectionStart, section);
    } else {
        content += "\n" + section;
    }

    std::ofstream outFile(userRegPath);
    if (!outFile.is_open()) return false;
    outFile << content;
    return true;
}

std::vector<DllInfo> DllManager::listDlls() {
    std::vector<DllInfo> result;
    std::vector<std::string> dirs = {getSystem32Path(), getSysWOW64Path()};

    for (auto& dir : dirs) {
        if (!std::filesystem::exists(dir)) continue;
        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".dll" || ext == ".drv" || ext == ".ocx") {
                    DllInfo info{};
                    info.name = entry.path().filename().string();
                    info.sizeBytes = entry.file_size();
                    info.exists = true;
                    info.path = entry.path().string();
                    auto it = overrides_.find(info.name);
                    info.overrideType = (it != overrides_.end()) ? it->second : DllOverrideType::Native;
                    result.push_back(info);
                }
            }
        }
    }
    return result;
}

std::optional<DllInfo> DllManager::getDllInfo(const std::string& dllName) {
    auto sys32 = std::filesystem::path(getSystem32Path()) / dllName;
    auto syswow64 = std::filesystem::path(getSysWOW64Path()) / dllName;

    auto checkPath = [&](const std::filesystem::path& p) -> std::optional<DllInfo> {
        if (std::filesystem::exists(p)) {
            DllInfo info{};
            info.name = dllName;
            info.path = p.string();
            info.sizeBytes = std::filesystem::file_size(p);
            info.exists = true;
            auto it = overrides_.find(dllName);
            info.overrideType = (it != overrides_.end()) ? it->second : DllOverrideType::Native;
            return info;
        }
        return std::nullopt;
    };

    if (auto info = checkPath(syswow64)) return info;
    return checkPath(sys32);
}

bool DllManager::installDll(const std::string& sourcePath, const std::string& targetDll) {
    if (!std::filesystem::exists(sourcePath)) return false;

    auto target = std::filesystem::path(getSystem32Path()) / targetDll;
    std::error_code ec;
    std::filesystem::copy_file(sourcePath, target, std::filesystem::copy_options::overwrite_existing, ec);
    return !ec;
}

bool DllManager::uninstallDll(const std::string& dllName) {
    auto sys32 = std::filesystem::path(getSystem32Path()) / dllName;
    auto syswow64 = std::filesystem::path(getSysWOW64Path()) / dllName;
    std::error_code ec;
    bool removed = false;
    if (std::filesystem::exists(sys32)) {
        std::filesystem::remove(sys32, ec);
        removed = !ec;
    }
    if (std::filesystem::exists(syswow64)) {
        std::filesystem::remove(syswow64, ec);
        removed = true;
    }
    return removed;
}

} // namespace fml
