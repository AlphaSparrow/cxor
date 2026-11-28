#include "Decoder.hpp"
#include "Encoder.hpp"
#include "Network.hpp"
#include "Node.hpp"
#include "Packet.hpp"
#include "Relay.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void testPacketConstruction() {
    cxor::Packet pkt(1, "A", "B", "HELLO");
    assert(pkt.id() == 1);
    assert(pkt.source() == "A");
    assert(pkt.destination() == "B");
    assert(pkt.size() == 5);
    assert(!pkt.isCoded());
    assert(pkt.payloadAsString() == "HELLO");
    assert(pkt.toHexString() == "48 45 4C 4C 4F");
}

void testXorBytesSameLength() {
    const std::vector<uint8_t> a{0xAA, 0x0F, 0x12};
    const std::vector<uint8_t> b{0x55, 0xF0, 0x12};
    const auto coded = cxor::Encoder::xorBytes(a, b);
    assert((coded == std::vector<uint8_t>{0xFF, 0xFF, 0x00}));

    // Self-inverse property: (A ^ B) ^ A == B
    const auto recoveredB = cxor::Encoder::xorBytes(coded, a);
    assert(recoveredB == b);
}

void testXorBytesDifferentLengths() {
    const std::vector<uint8_t> shorter{0x01, 0x02};
    const std::vector<uint8_t> longer{0x11, 0x22, 0x33, 0x44};
    const auto coded = cxor::Encoder::xorBytes(shorter, longer);
    assert(coded.size() == 4);
    assert((coded == std::vector<uint8_t>{0x10, 0x20, 0x33, 0x44}));
}

void testEncodeTwoPackets() {
    cxor::Packet pktA(101, "A", "B", "PING");
    cxor::Packet pktB(202, "B", "A", "PONG!!");

    const auto coded = cxor::Encoder::encode(900, "R", pktA, pktB);
    assert(coded.id() == 900);
    assert(coded.source() == "R");
    assert(coded.destination() == "broadcast");
    assert(coded.isCoded());
    assert(coded.size() == 6);
    assert(coded.constituents().size() == 2);
    assert(coded.constituents()[0].id == 101);
    assert(coded.constituents()[0].originalSize == 4);
    assert(coded.constituents()[1].id == 202);
    assert(coded.constituents()[1].originalSize == 6);

    // Verify XOR recovery manually before Decoder module exists
    auto recoveredA = cxor::Encoder::xorBytes(coded.payload(), pktB.payload());
    recoveredA.resize(coded.constituents()[0].originalSize);
    assert(recoveredA == pktA.payload());
}

void testEncodeMany() {
    std::vector<cxor::Packet> pkts{
        cxor::Packet(1, "A", "B", std::vector<uint8_t>{0x01, 0x02}),
        cxor::Packet(2, "B", "A", std::vector<uint8_t>{0x04, 0x08}),
        cxor::Packet(3, "C", "A", std::vector<uint8_t>{0x10, 0x20}),
    };

    const auto coded = cxor::Encoder::encodeMany(999, "R", pkts);
    assert(coded.isCoded());
    assert(coded.constituents().size() == 3);
    assert((coded.payload() == std::vector<uint8_t>{0x15, 0x2A}));

    bool threw = false;
    try {
        std::vector<cxor::Packet> single{pkts[0]};
        (void)cxor::Encoder::encodeMany(1000, "R", single);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void testDecodeTwoPackets() {
    cxor::Packet pktA(101, "A", "B", "PING");
    cxor::Packet pktB(202, "B", "A", "PONG!!");
    const auto coded = cxor::Encoder::encode(900, "R", pktA, pktB);

    const auto recoveredAtA = cxor::Decoder::decode(coded, pktA);
    assert(recoveredAtA == pktB);

    const auto recoveredAtB = cxor::Decoder::decode(coded, pktB);
    assert(recoveredAtB == pktA);
}

void testDecodeManyPackets() {
    cxor::Packet pktA(1, "A", "B", "ALPHA");
    cxor::Packet pktB(2, "B", "C", "BRAVO_2");
    cxor::Packet pktC(3, "C", "A", "CHARLIE");
    const std::vector<cxor::Packet> all{pktA, pktB, pktC};
    const auto coded = cxor::Encoder::encodeMany(901, "R", all);

    const std::vector<cxor::Packet> knownAtC{pktA, pktB};
    assert(cxor::Decoder::canDecode(coded, knownAtC));
    const auto recovered = cxor::Decoder::decode(coded, knownAtC);
    assert(recovered == pktC);
}

void testDecodeFailureCases() {
    cxor::Packet pktA(1, "A", "B", "ONE");
    cxor::Packet pktB(2, "B", "A", "TWO");
    const auto coded = cxor::Encoder::encode(902, "R", pktA, pktB);

    // No known packets -> cannot decode
    assert(!cxor::Decoder::canDecode(coded, {}));
    assert(!cxor::Decoder::tryDecode(coded, {}).has_value());

    // All packets already known -> nothing missing
    const std::vector<cxor::Packet> allKnown{pktA, pktB};
    assert(!cxor::Decoder::canDecode(coded, allKnown));

    // Native (uncoded) packet -> cannot decode
    assert(!cxor::Decoder::canDecode(pktA, {}));

    bool threw = false;
    try {
        (void)cxor::Decoder::decode(coded, std::span<const cxor::Packet>{});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void testNodeSendReceiveAndDecode() {
    cxor::Node nodeA("A");
    cxor::Node nodeB("B");

    cxor::Packet pktA(10, "A", "B", "FROM_A");
    cxor::Packet pktB(20, "B", "A", "FROM_B!!");

    nodeA.enqueuePacket(pktA);
    nodeB.enqueuePacket(pktB);

    assert(nodeA.hasOutgoing());
    assert(nodeB.hasOutgoing());
    assert(nodeA.hasKnownPacket(10));
    assert(nodeB.hasKnownPacket(20));

    const auto sentA = nodeA.popOutgoing();
    const auto sentB = nodeB.popOutgoing();
    assert(sentA.has_value() && sentB.has_value());
    assert(!nodeA.hasOutgoing());

    const auto coded = cxor::Encoder::encode(500, "R", *sentA, *sentB);

    assert(nodeA.receive(coded));
    assert(nodeB.receive(coded));

    assert(nodeA.deliveredPackets().size() == 1);
    assert(nodeA.deliveredPackets()[0] == pktB);

    assert(nodeB.deliveredPackets().size() == 1);
    assert(nodeB.deliveredPackets()[0] == pktA);
}

void testRelayCodingAndNormalForwarding() {
    cxor::Node nodeA("A");
    cxor::Node nodeB("B");
    cxor::Relay relay("R", 1000);

    cxor::Packet pktA(1, "A", "B", "MSG_FOR_B");
    cxor::Packet pktB(2, "B", "A", "MSG_FOR_A");
    cxor::Packet pktExtra(3, "A", "C", "UNICAST");

    nodeA.enqueuePacket(pktA);
    nodeB.enqueuePacket(pktB);
    nodeA.enqueuePacket(pktExtra);

    relay.receive(*nodeA.popOutgoing());
    relay.receive(*nodeB.popOutgoing());
    relay.receive(*nodeA.popOutgoing());
    assert(relay.pendingCount() == 3);

    // First transmission: coded (pktA ^ pktB)
    const auto tx1 = relay.forwardNext(true);
    assert(tx1.has_value());
    assert(tx1->isCoded());
    assert(tx1->id() == 1000);
    assert(nodeA.receive(*tx1));
    assert(nodeB.receive(*tx1));
    assert(nodeA.deliveredPackets().back() == pktB);
    assert(nodeB.deliveredPackets().back() == pktA);

    // Second transmission: uncoded fallback (pktExtra)
    const auto tx2 = relay.forwardNext(true);
    assert(tx2.has_value());
    assert(!tx2->isCoded());
    assert(*tx2 == pktExtra);
    assert(!relay.hasPending());

    assert(relay.codedTransmissions() == 1);
    assert(relay.uncodedTransmissions() == 1);
    assert(relay.totalTransmissions() == 2);
}

void testNetworkNormalVsCodedComparison() {
    const std::vector<cxor::TrafficEvent> bidirectionalTraffic{
        {"A", "B", "PKT_A1"},
        {"B", "A", "PKT_B1"},
        {"A", "B", "PKT_A2"},
        {"B", "A", "PKT_B2"},
    };

    const auto report =
        cxor::Network::compareScenario("Symmetric Bidirectional", bidirectionalTraffic);

    assert(report.normal.packetsSent == 4);
    assert(report.normal.packetsDelivered == 4);
    assert(report.normal.relayTransmissions() == 4);
    assert(report.normal.totalTransmissions() == 8);

    assert(report.coded.packetsSent == 4);
    assert(report.coded.packetsDelivered == 4);
    assert(report.coded.relayCodedTransmissions == 2);
    assert(report.coded.relayUncodedTransmissions == 0);
    assert(report.coded.totalTransmissions() == 6);

    assert(report.transmissionsSaved() == 2);
    assert(report.savingsPercent() == 25.0);
}

}  // namespace

int main() {
    testPacketConstruction();
    testXorBytesSameLength();
    testXorBytesDifferentLengths();
    testEncodeTwoPackets();
    testEncodeMany();
    testDecodeTwoPackets();
    testDecodeManyPackets();
    testDecodeFailureCases();
    testNodeSendReceiveAndDecode();
    testRelayCodingAndNormalForwarding();
    testNetworkNormalVsCodedComparison();

    std::cout << "All Module 1-5 (Packet, Encoder, Decoder, Node, Relay, Network) tests passed.\n";
    return 0;
}
