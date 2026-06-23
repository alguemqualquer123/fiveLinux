#pragma once

#include <string>
#include <optional>
#include <vector>

#include "fivemlinux/types.h"

namespace fml {

enum class RockstarAuthStatus {
    Unknown,
    NotInstalled,
    Installed,
    LoggedIn,
    LoggedOut,
    Error,
    NeedsUpdate
};

struct RockstarInstallInfo {
    bool installed;
    std::string installPath;
    std::string launcherPath;
    std::string version;
    uint64_t totalSizeBytes;
    RockstarAuthStatus status;
};

struct RockstarAccount {
    std::string nickname;
    std::string email;
    bool isOnline;
    std::string token;
};

struct RockstarLauncherState {
    RockstarAuthStatus authStatus;
    bool launcherRunning;
    bool socialClubRunning;
    bool cloudSyncEnabled;
    std::string installDir;
    std::string prefixPath;
    RockstarAccount account;
};

class RockstarLauncher {
public:
    RockstarLauncher() = default;

    RockstarInstallInfo detect() const;
    bool isInstalled() const;
    RockstarLauncherState getState() const;

    bool launch(const std::string& winePath = "wine");
    bool stop();
    bool isRunning() const;

    bool login(const std::string& email, const std::string& password);
    bool loginWithSteam();
    bool logout();
    bool isOnline() const;

    bool configurePrefix(const std::string& prefixPath);
    bool installDependencies(const std::string& prefixPath);

    std::string getLauncherPath() const;
    std::string getDefaultPrefix() const;

    static std::vector<std::string> getSearchPaths();
    static std::string getSocialClubRegistryPath();

private:
    bool runRockstarCommand(const std::string& winePath,
                            const std::string& prefixPath,
                            const std::string& args);
    bool setupWineEnvironment(const std::string& prefixPath);
    bool installSocialClub(const std::string& prefixPath);
    bool installLauncher(const std::string& prefixPath);
    bool checkSocialClubVersion(const std::string& prefixPath);

    pid_t pid_ = 0;
};

} // namespace fml
