#pragma once

#include <string>
#include <filesystem>

namespace fml {

class Environment {
public:
    static std::string homeDir();
    static std::string configDir();
    static std::string cacheDir();
    static std::string dataDir();
    static std::string logDir();
    static std::string tempDir();

    static std::string getEnv(const std::string& name);
    static void setEnv(const std::string& name, const std::string& value);
    static std::string expandPath(const std::string& path);

private:
    Environment() = delete;
};

} // namespace fml