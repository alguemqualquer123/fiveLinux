#include "registry.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace fml {

WineRegistry::WineRegistry(const std::string& prefixPath)
    : prefixPath_(prefixPath) {}

std::string WineRegistry::hiveFilePath(RegistryHive hive) const {
    switch (hive) {
        case RegistryHive::HKLM: return prefixPath_ + "/system.reg";
        case RegistryHive::HKCU: return prefixPath_ + "/user.reg";
        default: return prefixPath_ + "/system.reg";
    }
}

bool WineRegistry::load(RegistryHive hive) {
    std::string path = hiveFilePath(hive);
    if (!std::filesystem::exists(path)) return false;

    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    std::string currentKey;
    bool inData = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';') continue;

        if (line.substr(0, 5) == "WINE ") continue;

        if (line[0] == '[') {
            auto end = line.find(']');
            if (end != std::string::npos) {
                currentKey = line.substr(1, end - 1);
                inData = true;
            }
            continue;
        }

        if (inData && !currentKey.empty()) {
            auto eq = line.find('=');
            if (eq != std::string::npos) {
                std::string valueName = line.substr(0, eq);
                std::string rawValue = line.substr(eq + 1);
                RegistryValue val{};
                val.rawValue = rawValue;

                if (rawValue.size() > 2 && rawValue.substr(0, 2) == "dword:") {
                    val.type = RegistryValueType::Dword;
                    val.dwordValue = std::stoul(rawValue.substr(2), nullptr, 16);
                } else if (rawValue.size() > 6 && rawValue.substr(0, 6) == "str(2):") {
                    val.type = RegistryValueType::String;
                } else if (rawValue.size() > 4 && rawValue.substr(0, 4) == "str:") {
                    val.type = RegistryValueType::String;
                } else if (rawValue.size() > 7 && rawValue.substr(0, 7) == "hex(2):") {
                    val.type = RegistryValueType::ExpandString;
                } else if (rawValue.size() > 4 && rawValue.substr(0, 4) == "hex:") {
                    val.type = RegistryValueType::Binary;
                } else {
                    val.type = RegistryValueType::String;
                }

                entries_[currentKey][valueName] = val;
            }
        }
    }

    return true;
}

bool WineRegistry::save(RegistryHive hive) {
    std::string path = hiveFilePath(hive);
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "WINE REGISTRY Version 2\n";
    file << ";; All keys relative to \\\\User\\\\CurrentUser\n\n";

    for (auto& [key, values] : entries_) {
        file << "[" << key << "]\n";
        for (auto& [name, val] : values) {
            file << "\"" << name << "\"" << "=" << val.rawValue << "\n";
        }
        file << "\n";
    }

    return true;
}

std::optional<RegistryValue> WineRegistry::get(const std::string& key,
                                                const std::string& valueName) {
    auto keyIt = entries_.find(key);
    if (keyIt == entries_.end()) return std::nullopt;

    auto valIt = keyIt->second.find(valueName);
    if (valIt == keyIt->second.end()) return std::nullopt;

    return valIt->second;
}

bool WineRegistry::set(const std::string& key,
                        const std::string& valueName,
                        const RegistryValue& value) {
    entries_[key][valueName] = value;
    return true;
}

bool WineRegistry::remove(const std::string& key, const std::string& valueName) {
    auto keyIt = entries_.find(key);
    if (keyIt == entries_.end()) return false;

    auto valIt = keyIt->second.find(valueName);
    if (valIt == keyIt->second.end()) return false;

    keyIt->second.erase(valIt);
    if (keyIt->second.empty()) entries_.erase(keyIt);
    return true;
}

std::vector<std::string> WineRegistry::listKeys(const std::string& prefix) {
    std::vector<std::string> result;
    for (auto& [key, _] : entries_) {
        if (prefix.empty() || key.find(prefix) == 0) {
            result.push_back(key);
        }
    }
    return result;
}

} // namespace fml
