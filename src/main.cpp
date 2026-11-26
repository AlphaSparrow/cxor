#include "Node.hpp"
#include "Packet.hpp"
#include "Relay.hpp"

#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  CXOR - Wireless Network Coding (C++20)\n";
    std::cout << "  Node A <-> Relay R <-> Node B\n";
    std::cout << "========================================\n\n";

    cxor::Node nodeA("A");
    cxor::Node nodeB("B");
    cxor::Relay relay("R", 100);

    nodeA.enqueuePacket(cxor::Packet(1, "A", "B", "HELLO_B"));
    nodeB.enqueuePacket(cxor::Packet(2, "B", "A", "WORLD_A!"));

    if (auto pkt = nodeA.popOutgoing()) {
        std::cout << "[Node A -> Relay R] Packet #" << pkt->id()
                  << " | \"" << pkt->payloadAsString() << "\"\n";
        relay.receive(std::move(*pkt));
    }

    if (auto pkt = nodeB.popOutgoing()) {
        std::cout << "[Node B -> Relay R] Packet #" << pkt->id()
                  << " | \"" << pkt->payloadAsString() << "\"\n";
        relay.receive(std::move(*pkt));
    }

    if (auto tx = relay.forwardNext(true)) {
        std::cout << "[Relay R -> Broadcast] "
                  << (tx->isCoded() ? "Coded" : "Native")
                  << " Packet #" << tx->id()
                  << " (" << tx->size() << " B) | HEX: " << tx->toHexString() << '\n';

        nodeA.receive(*tx);
        nodeB.receive(*tx);
    }

    std::cout << "\n--- Decoded Deliveries ---\n";
    for (const auto& pkt : nodeA.deliveredPackets()) {
        std::cout << "[Node A] Received Packet #" << pkt.id()
                  << " from " << pkt.source() << ": \"" << pkt.payloadAsString() << "\"\n";
    }
    for (const auto& pkt : nodeB.deliveredPackets()) {
        std::cout << "[Node B] Received Packet #" << pkt.id()
                  << " from " << pkt.source() << ": \"" << pkt.payloadAsString() << "\"\n";
    }

    std::cout << "\nRelay transmissions: " << relay.totalTransmissions()
              << " (coded: " << relay.codedTransmissions()
              << ", uncoded: " << relay.uncodedTransmissions() << ")\n";

    return 0;
}
