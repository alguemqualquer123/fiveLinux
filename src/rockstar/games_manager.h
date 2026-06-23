#pragma once

#include <string>
#include <vector>
#include <map>

#include "fivemlinux/types.h"

namespace fml {

enum class RockstarGame {
    GTAV,
    RDR2,
    MaxPayne3,
    LANoire,
    Bully,
    Unknown
};

struct GameLicense {
    RockstarGame game;
    std::string name;
    std::string installPath;
    bool installed;
    bool ownsLicense;
    std::string lastPlayed;
    uint64_t playTimeSeconds;
};

class RockstarGamesManager {
public:
    RockstarGamesManager() = default;

    std::vector<GameLicense> getOwnedGames() const;
    std::vector<GameLicense> getInstalledGames() const;
    std::optional<GameLicense> getGameInfo(RockstarGame game) const;

    bool isGameOwned(RockstarGame game) const;
    bool isGameInstalled(RockstarGame game) const;

    std::string getGameInstallPath(RockstarGame game) const;
    std::string getGameExePath(RockstarGame game) const;

    bool repairGame(RockstarGame game);
    bool validateGameFiles(RockstarGame game);

    static std::string gameToString(RockstarGame game);
    static RockstarGame stringToGame(const std::string& name);

private:
    std::vector<std::string> getGameSearchPaths(RockstarGame game) const;
    bool validateGtaV(const std::string& path) const;
    bool validateRdr2(const std::string& path) const;

    std::map<RockstarGame, GameLicense> licenses_;
};

} // namespace fml
