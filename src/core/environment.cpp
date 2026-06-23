#include "environment.h"

#include <cstdlib>
#include <algorithm>

namespace fml {

std::string Environment::homeDir() {
    std::string home = getEnv("HOME");
    if (home.empty()) {
        home = getEnv("USERPROFILE");
    }
    return home;
}

std::string Environment::configDir() {
    std::string xdgConfig = getEnv("XDG_CONFIG_HOME");
    if (!xdgConfig.empty()) {
        return xdgConfig;
    }
    return homeDir() + "/.config";
}

std::string Environment::cacheDir() {
    std::string xdgCache = getEnv("XDG_CACHE_HOME");
    if (!xdgCache.empty()) {
        return xdgCache;
    }
    return homeDir() + "/.cache";
}

std::string Environment::dataDir() {
    std::string xdgData = getEnv("XDG_DATA_HOME");
    if (!xdgData.empty()) {
        return xdgData;
    }
    return homeDir() + "/.local/share";
}

std::string Environment::logDir() {
    std::string xdgLog = getEnv("XDG_STATE_HOME");
    if (!xdgLog.empty()) {
        return xdgLog + "/log";
    }
    return homeDir() + "/.local/state/log";
}

std::string Environment::tempDir() {
    std::string tmp = getEnv("TMPDIR");
    if (tmp.empty()) {
        tmp = getEnv("TEMP");
    }
    if (tmp.empty()) {
        tmp = getEnv("TMP");
    }
    if (tmp.empty()) {
        tmp = "/tmp";
    }
    return tmp;
}

std::string Environment::getEnv(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : "";
}

void Environment::setEnv(const std::string& name, const std::string& value) {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

std::string Environment::expandPath(const std::string& path) {
    if (path.empty()) return path;

    std::string result = path;

    if (result[0] == '~') {
        std::string home = homeDir();
        if (result.size() > 1 && result[1] == '/') {
            result = home + result.substr(1);
        } else if (result.size() == 1) {
            result = home;
        }
    }

    std::string envPrefix = "$(";
    size_t pos = 0;
    while ((pos = result.find(envPrefix, pos)) != std::string::npos) {
        size_t endPos = result.find(')', pos);
        if (endPos == std::string::npos) break;

        std::string envName = result.substr(pos + 2, endPos - pos - 2);
        std::string envValue = getEnv(envName);
        result.replace(pos, endPos - pos + 1, envValue);
        pos += envValue.size();
    }

    return result;
}

} // namespace fml