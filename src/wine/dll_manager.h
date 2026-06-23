#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>

#include "fivemlinux/types.h"

namespace fml {

enum class DllOverrideType {
    Builtin,
    Native,
    Disabled
};

struct DllInfo {
    std::string name;
    DllOverrideType overrideType;
    std::string path;
    uint64_t sizeBytes;
    bool exists;
};

class DllManager {
public:
    explicit DllManager(const std::string& prefixPath);

    std::map<std::string, DllOverrideType> getDllOverrides();
    bool setDllOverride(const std::string& dllName, DllOverrideType type);
    bool removeDllOverride(const std::string& dllName);
    std::vector<DllInfo> listDlls();
    std::optional<DllInfo> getDllInfo(const std::string& dllName);
    bool installDll(const std::string& sourcePath, const std::string& targetDll);
    bool uninstallDll(const std::string& dllName);

private:
    std::string getSystem32Path() const;
    std::string getSysWOW64Path() const;
    bool parseDllOverrides();
    bool writeDllOverrides();

    std::string prefixPath_;
    std::map<std::string, DllOverrideType> overrides_;
};

} // namespace fml
