#include "Network.hpp"
#include "Packet.hpp"

#include <unordered_set>
#include <utility>

namespace cxor {

Network::Network(std::string relayId) : relay_(std::move(relayId)) {}

void Network::addNode(std::string nodeId) {
    if (nodes_.contains(nodeId)) {
        return;
    }
    nodeOrder_.push_back(nodeId);
    nodes_.emplace(nodeId, Node(nodeId));
}

bool Network::sendPacket(const std::string& source,
                         const std::string& destination,
                         std::string_view payload) {
    auto srcIt = nodes_.find(source);
    if (srcIt == nodes_.end() || !nodes_.contains(destination) || source == destination) {
        return false;
    }

    srcIt->second.enqueuePacket(Packet(nextPacketId_++, source, destination, payload));
    ++packetsSent_;
    return true;
}

SimulationMetrics Network::run(std::string scenarioName, bool codingEnabled) {
    SimulationMetrics metrics;
    metrics.scenarioName = std::move(scenarioName);
    metrics.codingEnabled = codingEnabled;
    metrics.packetsSent = packetsSent_;

    // Phase 1: Drain node outboxes round-robin to the relay (uplink).
    bool anyOutgoing = true;
    while (anyOutgoing) {
        anyOutgoing = false;
        for (const auto& nodeId : nodeOrder_) {
            auto& node = nodes_.at(nodeId);
            if (auto pkt = node.popOutgoing()) {
                anyOutgoing = true;
                ++metrics.uplinkTransmissions;
                relay_.receive(std::move(*pkt));
            }
        }
    }

    // Phase 2: Relay forwards packets (coded when possible) and nodes receive.
    while (relay_.hasPending()) {
        if (auto tx = relay_.forwardNext(codingEnabled)) {
            for (const auto& nodeId : nodeOrder_) {
                nodes_.at(nodeId).receive(*tx);
            }
        }
    }

    for (const auto& nodeId : nodeOrder_) {
        metrics.packetsDelivered += nodes_.at(nodeId).deliveredPackets().size();
    }
    metrics.relayUncodedTransmissions = relay_.uncodedTransmissions();
    metrics.relayCodedTransmissions = relay_.codedTransmissions();
    return metrics;
}

ComparisonReport Network::compareScenario(std::string scenarioName,
                                          const std::vector<TrafficEvent>& traffic) {
    auto runMode = [&](bool codingEnabled) {
        Network net("R");
        std::unordered_set<std::string> seenNodes;
        for (const auto& event : traffic) {
            if (seenNodes.insert(event.source).second) {
                net.addNode(event.source);
            }
            if (seenNodes.insert(event.destination).second) {
                net.addNode(event.destination);
            }
        }
        for (const auto& event : traffic) {
            net.sendPacket(event.source, event.destination, event.payload);
        }
        return net.run(scenarioName, codingEnabled);
    };

    return ComparisonReport{
        .normal = runMode(false),
        .coded = runMode(true),
    };
}

}  // namespace cxor
