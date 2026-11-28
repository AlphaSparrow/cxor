#pragma once

#include "Node.hpp"
#include "Relay.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cxor {

struct TrafficEvent {
    std::string source;
    std::string destination;
    std::string payload;
};

struct SimulationMetrics {
    std::string scenarioName;
    bool codingEnabled{false};
    uint64_t packetsSent{0};
    uint64_t packetsDelivered{0};
    uint64_t uplinkTransmissions{0};
    uint64_t relayUncodedTransmissions{0};
    uint64_t relayCodedTransmissions{0};

    [[nodiscard]] uint64_t relayTransmissions() const noexcept {
        return relayUncodedTransmissions + relayCodedTransmissions;
    }

    [[nodiscard]] uint64_t totalTransmissions() const noexcept {
        return uplinkTransmissions + relayTransmissions();
    }

    [[nodiscard]] double deliveryRatio() const noexcept {
        return packetsSent == 0
                   ? 0.0
                   : static_cast<double>(packetsDelivered) / static_cast<double>(packetsSent);
    }
};

struct ComparisonReport {
    SimulationMetrics normal;
    SimulationMetrics coded;

    [[nodiscard]] uint64_t transmissionsSaved() const noexcept {
        if (normal.totalTransmissions() <= coded.totalTransmissions()) {
            return 0;
        }
        return normal.totalTransmissions() - coded.totalTransmissions();
    }

    [[nodiscard]] double savingsPercent() const noexcept {
        if (normal.totalTransmissions() == 0) {
            return 0.0;
        }
        return (static_cast<double>(transmissionsSaved()) * 100.0) /
               static_cast<double>(normal.totalTransmissions());
    }
};

class Network {
public:
    explicit Network(std::string relayId = "R");

    void addNode(std::string nodeId);
    bool sendPacket(const std::string& source,
                    const std::string& destination,
                    std::string_view payload);

    [[nodiscard]] SimulationMetrics run(std::string scenarioName, bool codingEnabled);

    [[nodiscard]] static ComparisonReport compareScenario(
        std::string scenarioName,
        const std::vector<TrafficEvent>& traffic);

private:
    Relay relay_;
    std::unordered_map<std::string, Node> nodes_;
    std::vector<std::string> nodeOrder_;
    uint64_t nextPacketId_{1};
    uint64_t packetsSent_{0};
};

}  // namespace cxor
