#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

struct PingResult {
    std::string host;
    double latencyMs;
    bool reachable;
    int packetsSent;
    int packetsReceived;
    double packetLoss;
};

class LatencyTest {
public:
    LatencyTest() = default;

    PingResult testPing(const std::string& host, int count = 4) const;
    std::vector<PingResult> testFiveMServers() const;
    std::string getBestServer() const;

private:
    PingResult parsePingOutput(const std::string& output, const std::string& host) const;
};

} // namespace fml
