#include "Encoder.hpp"
#include "Packet.hpp"

#include <iostream>

int main() {
    std::cout << "========================================\n";
    std::cout << "  CXOR - Wireless Network Coding (C++20)\n";
    std::cout << "  Module 1: Packet & XOR Engine\n";
    std::cout << "========================================\n\n";

    cxor::Packet pktA(1, "A", "B", "HELLO_B");
    cxor::Packet pktB(2, "B", "A", "WORLD_A!");

    cxor::Packet coded = cxor::Encoder::encode(100, "R", pktA, pktB);

    std::cout << "[Node A -> Relay R] Packet #" << pktA.id()
              << " (" << pktA.size() << " B) | \"" << pktA.payloadAsString()
              << "\" | HEX: " << pktA.toHexString() << '\n';

    std::cout << "[Node B -> Relay R] Packet #" << pktB.id()
              << " (" << pktB.size() << " B) | \"" << pktB.payloadAsString()
              << "\" | HEX: " << pktB.toHexString() << '\n';

    std::cout << "[Relay R -> Broadcast] Coded Packet #" << coded.id()
              << " (" << coded.size() << " B) | HEX: " << coded.toHexString() << '\n';

    return 0;
}
