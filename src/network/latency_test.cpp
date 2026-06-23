#include "latency_test.h"

#include <sstream>
#include <cstdlib>

namespace fml {

PingResult LatencyTest::parsePingOutput(const std::string& output,
                                         const std::string& host) const {
    PingResult result{};
    result.host = host;
    result.reachable = false;
    result.latencyMs = -1.0;
    result.packetsSent = 4;
    result.packetsReceived = 0;
    result.packetLoss = 100.0;

    std::istringstream stream(output);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("packet loss") != std::string::npos) {
            size_t pos = line.find("%");
            if (pos != std::string::npos) {
                size_t start = line.rfind(' ', pos - 1);
                if (start != std::string::npos) {
                    try {
                        result.packetLoss = std::stod(line.substr(start + 1, pos - start - 1));
                    } catch (...) {}
                }
            }
        }

        if (line.find("rtt") != std::string::npos || line.find("round-trip") != std::string::npos) {
            size_t slash1 = line.find('/');
            if (slash1 != std::string::npos) {
                size_t slash2 = line.find('/', slash1 + 1);
                if (slash2 != std::string::npos) {
                    try {
                        result.latencyMs = std::stod(line.substr(slash1 + 1, slash2 - slash1 - 1));
                    } catch (...) {}
                }
            }
        }

        if (line.find("bytes from") != std::string::npos) {
            result.packetsReceived++;
            result.reachable = true;
        }
    }

    return result;
}

PingResult LatencyTest::testPing(const std::string& host, int count) const {
    std::string cmd = "ping -c " + std::to_string(count) + " -W 3 " + host + " 2>&1";
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        PingResult result{};
        result.host = host;
        result.reachable = false;
        return result;
    }

    std::ostringstream output;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output << buffer;
    }
    pclose(pipe);

    return parsePingOutput(output.str(), host);
}

std::vector<PingResult> LatencyTest::testFiveMServers() const {
    std::vector<std::string> hosts = {
        "runtime.fivem.net",
        "servers-frontend.fivem.net",
        "api.fivem.net",
        "cdn.fivem.net",
        "play.gta5-fivem.net"
    };

    std::vector<PingResult> results;
    for (auto& host : hosts) {
        results.push_back(testPing(host, 3));
    }
    return results;
}

std::string LatencyTest::getBestServer() const {
    auto results = testFiveMServers();
    std::string bestHost;
    double bestLatency = 999999.0;

    for (auto& r : results) {
        if (r.reachable && r.latencyMs >= 0 && r.latencyMs < bestLatency) {
            bestLatency = r.latencyMs;
            bestHost = r.host;
        }
    }

    return bestHost;
}

} // namespace fml
