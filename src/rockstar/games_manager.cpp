#include "games_manager.h"

#include <cstdlib>
#include <fstream>

namespace fml {

std::string RockstarGamesManager::gameToString(RockstarGame game) {
    switch (game) {
        case RockstarGame::GTAV: return "Grand Theft Auto V";
        case RockstarGame::RDR2: return "Red Dead Redemption 2";
        case RockstarGame::MaxPayne3: return "Max Payne 3";
        case RockstarGame::L.A.Noire: return "L.A. Noire";
        case RockstarGame::Bully: return "Bully";
        default: return "Unknown";
    }
}

RockstarGame RockstarGamesManager::stringToGame(const std::string& name) {
    if (name.find("GTA") != std::string::npos || name.find("Grand Theft") != std::string::npos)
        return RockstarGame::GTAV;
    if (name.find("RDR") != std::string::npos || name.find("Red Dead") != std::string::npos)
        return RockstarGame::RDR2;
    if (name.find("Max Payne") != std::string::npos)
        return RockstarGame::MaxPayne3;
    if (name.find("L.A. Noire") != std::string::npos || name.find("LANoire") != std::string::npos)
        return RockstarGame::L.A.Noire;
    if (name.find("Bully") != std::string::npos)
        return RockstarGame::Bully;
    return RockstarGame::Unknown;
}

std::vector<std::string> RockstarGamesManager::getGameSearchPaths(RockstarGame game) const {
    const char* home = getenv("HOME");
    if (!home) return {};

    std::string h(home);
    std::vector<std::string> paths;

    std::string gameName;
    switch (game) {
        case RockstarGame::GTAV:
            gameName = "Grand Theft Auto V";
            break;
        case RockstarGame::RDR2:
            gameName = "Red Dead Redemption 2";
            break;
        case RockstarGame::MaxPayne3:
            gameName = "Max Payne 3";
            break;
        case RockstarGame::L.A.Noire:
            gameName = "L.A. Noire";
            break;
        case RockstarGame::Bully:
            gameName = "Bully";
            break;
        default:
            return {};
    }

    paths.push_back(h + "/Games/Rockstar Games/" + gameName);
    paths.push_back(h + "/.wine/drive_c/Program Files/Rockstar Games/" + gameName);
    paths.push_back(h + "/.wine/drive_c/Program Files (x86)/Rockstar Games/" + gameName);
    paths.push_back(h + "/.local/share/Steam/steamapps/common/" + gameName);

    auto steamfolders = h + "/.steam/steam/steamapps/libraryfolders.vdf";
    if (std::filesystem::exists(steamfolders)) {
        std::ifstream vdf(steamfolders);
        std::string line;
        while (std::getline(vdf, line)) {
            size_t pos = line.find("\"path\"");
            if (pos != std::string::npos) {
                auto start = line.find('"', pos + 6);
                auto end = line.find('"', start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string libPath = line.substr(start + 1, end - start - 1);
                    paths.push_back(libPath + "/steamapps/common/" + gameName);
                }
            }
        }
    }

    return paths;
}

bool RockstarGamesManager::validateGtaV(const std::string& path) const {
    if (!std::filesystem::exists(path)) return false;

    bool hasExe = std::filesystem::exists(std::filesystem::path(path) / "GTA5.exe");
    bool hasRpf = false;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        if (name.find("x64") == 0 && name.find(".rpf") != std::string::npos) {
            hasRpf = true;
            break;
        }
    }
    return hasExe && hasRpf;
}

bool RockstarGamesManager::validateRdr2(const std::string& path) const {
    if (!std::filesystem::exists(path)) return false;
    return std::filesystem::exists(std::filesystem::path(path) / "RDR2.exe") ||
           std::filesystem::exists(std::filesystem::path(path) / "PlayRDR2.exe");
}

std::optional<GameLicense> RockstarGamesManager::getGameInfo(RockstarGame game) const {
    GameLicense license{};
    license.game = game;
    license.name = gameToString(game);
    license.installed = false;
    license.ownsLicense = false;

    auto paths = getGameSearchPaths(game);

    for (auto& path : paths) {
        bool valid = false;
        switch (game) {
            case RockstarGame::GTAV: valid = validateGtaV(path); break;
            case RockstarGame::RDR2: valid = validateRdr2(path); break;
            default:
                valid = std::filesystem::exists(path);
                break;
        }

        if (valid) {
            license.installed = true;
            license.installPath = path;
            license.ownsLicense = true;

            uint64_t totalSize = 0;
            for (auto& entry : std::filesystem::recursive_directory_iterator(path,
                    std::filesystem::directory_options::skip_permission_denied)) {
                if (entry.is_regular_file()) {
                    std::error_code ec;
                    totalSize += entry.file_size(ec);
                }
            }
            break;
        }
    }

    return license;
}

std::vector<GameLicense> RockstarGamesManager::getOwnedGames() const {
    std::vector<GameLicense> games;

    std::vector<RockstarGame> allGames = {
        RockstarGame::GTAV,
        RockstarGame::RDR2,
        RockstarGame::MaxPayne3,
        RockstarGame::L.A.Noire,
        RockstarGame::Bully
    };

    for (auto& game : allGames) {
        auto info = getGameInfo(game);
        if (info) games.push_back(*info);
    }

    return games;
}

std::vector<GameLicense> RockstarGamesManager::getInstalledGames() const {
    std::vector<GameLicense> installed;
    auto all = getOwnedGames();
    for (auto& game : all) {
        if (game.installed) installed.push_back(game);
    }
    return installed;
}

bool RockstarGamesManager::isGameOwned(RockstarGame game) const {
    auto info = getGameInfo(game);
    return info && info->ownsLicense;
}

bool RockstarGamesManager::isGameInstalled(RockstarGame game) const {
    auto info = getGameInfo(game);
    return info && info->installed;
}

std::string RockstarGamesManager::getGameInstallPath(RockstarGame game) const {
    auto info = getGameInfo(game);
    return (info && info->installed) ? info->installPath : "";
}

std::string RockstarGamesManager::getGameExePath(RockstarGame game) const {
    auto path = getGameInstallPath(game);
    if (path.empty()) return "";

    switch (game) {
        case RockstarGame::GTAV: {
            auto exe = std::filesystem::path(path) / "GTA5.exe";
            return std::filesystem::exists(exe) ? exe.string() : "";
        }
        case RockstarGame::RDR2: {
            auto exe = std::filesystem::path(path) / "RDR2.exe";
            if (std::filesystem::exists(exe)) return exe.string();
            auto play = std::filesystem::path(path) / "PlayRDR2.exe";
            return std::filesystem::exists(play) ? play.string() : "";
        }
        default: return "";
    }
}

bool RockstarGamesManager::repairGame(RockstarGame /*game*/) {
    return true;
}

bool RockstarGamesManager::validateGameFiles(RockstarGame /*game*/) {
    return true;
}

} // namespace fml
