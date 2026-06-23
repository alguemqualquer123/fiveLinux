#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <chrono>

#include "fivemlinux/types.h"

namespace fml {

struct CliCommand {
    std::string name;
    std::string description;
    std::string usage;
    std::function<int(const std::vector<std::string>&)> handler;
};

class CommandInterface {
public:
    CommandInterface();

    int run(int argc, char* argv[]);
    void registerCommand(const CliCommand& cmd);

private:
    void registerBuiltinCommands();
    std::vector<std::string> parseArgs(int argc, char* argv[]);

    void printBanner();
    void printStars();
    void printMeteor();
    void printLoadingBar(float progress, const std::string& label);
    void printSeparator();
    void printHeader(const std::string& title);
    void printSuccess(const std::string& msg);
    void printError(const std::string& msg);
    void printWarning(const std::string& msg);
    void printInfo(const std::string& msg);
    void printCheck(bool ok, const std::string& label, const std::string& detail = "");

    int cmdStatus(const std::vector<std::string>& args);
    int cmdRepair(const std::vector<std::string>& args);
    int cmdInstall(const std::vector<std::string>& args);
    int cmdDiagnose(const std::vector<std::string>& args);
    int cmdWine(const std::vector<std::string>& args);
    int cmdCache(const std::vector<std::string>& args);
    int cmdLaunch(const std::vector<std::string>& args);
    int cmdGpu(const std::vector<std::string>& args);
    int cmdServer(const std::vector<std::string>& args);
    int cmdNetwork(const std::vector<std::string>& args);
    int cmdRockstar(const std::vector<std::string>& args);
    int cmdVersion(const std::vector<std::string>& args);

    std::map<std::string, CliCommand> commands_;
};

} // namespace fml
