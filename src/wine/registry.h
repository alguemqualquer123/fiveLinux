#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

#include "fivemlinux/types.h"

namespace fml {

enum class RegistryHive {
    HKLM,
    HKCU,
    HKCR,
    HKU,
    HKCC
};

enum class RegistryValueType {
    String,
    Dword,
    Qword,
    Binary,
    MultiString,
    ExpandString
};

struct RegistryValue {
    RegistryValueType type;
    std::string rawValue;
    std::optional<uint32_t> dwordValue;
    std::optional<uint64_t> qwordValue;
};

class WineRegistry {
public:
    explicit WineRegistry(const std::string& prefixPath);

    bool load(RegistryHive hive);
    bool save(RegistryHive hive);
    bool exportTo(const std::string& path, RegistryHive hive);
    bool importFrom(const std::string& path);

    std::optional<RegistryValue> get(const std::string& key,
                                     const std::string& valueName);
    bool set(const std::string& key,
             const std::string& valueName,
             const RegistryValue& value);
    bool remove(const std::string& key, const std::string& valueName);
    std::vector<std::string> listKeys(const std::string& key);

private:
    std::string hiveFilePath(RegistryHive hive) const;
    RegistryHive parseHiveName(const std::string& name) const;
    std::vector<std::string> parseRegFile(const std::string& path);
    std::map<std::string, std::map<std::string, RegistryValue>> entries_;
    std::string prefixPath_;
};

} // namespace fml
