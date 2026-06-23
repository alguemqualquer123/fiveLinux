#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

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
    void printHelp() const;
    void printVersion() const;

private:
    void registerBuiltinCommands();
    std::vector<std::string> parseArgs(int argc, char* argv[]);

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

    std::map<std::string, CliCommand> commands_;
};

} // namespace fml
