#include "Encoder.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace cxor {

std::vector<uint8_t> Encoder::xorBytes(std::span<const uint8_t> a,
                                       std::span<const uint8_t> b) {
    const std::size_t maxLen = std::max(a.size(), b.size());
    std::vector<uint8_t> result(maxLen, 0);

    for (std::size_t i = 0; i < maxLen; ++i) {
        const uint8_t byteA = (i < a.size()) ? a[i] : 0;
        const uint8_t byteB = (i < b.size()) ? b[i] : 0;
        result[i] = static_cast<uint8_t>(byteA ^ byteB);
    }

    return result;
}

Packet Encoder::encode(uint64_t codedId,
                       std::string relayId,
                       const Packet& a,
                       const Packet& b) {
    std::vector<uint8_t> codedPayload = xorBytes(a.payload(), b.payload());

    std::vector<ConstituentInfo> constituents{
        ConstituentInfo{a.id(), a.source(), a.destination(), a.size()},
        ConstituentInfo{b.id(), b.source(), b.destination(), b.size()},
    };

    return Packet::makeCoded(codedId,
                             std::move(relayId),
                             "broadcast",
                             std::move(codedPayload),
                             std::move(constituents));
}

Packet Encoder::encodeMany(uint64_t codedId,
                           std::string relayId,
                           std::span<const Packet> packets) {
    if (packets.size() < 2) {
        throw std::invalid_argument("Encoder::encodeMany requires at least 2 packets");
    }

    std::vector<uint8_t> codedPayload = packets[0].payload();
    std::vector<ConstituentInfo> constituents;
    constituents.reserve(packets.size());
    constituents.push_back(ConstituentInfo{
        packets[0].id(),
        packets[0].source(),
        packets[0].destination(),
        packets[0].size(),
    });

    for (std::size_t i = 1; i < packets.size(); ++i) {
        codedPayload = xorBytes(codedPayload, packets[i].payload());
        constituents.push_back(ConstituentInfo{
            packets[i].id(),
            packets[i].source(),
            packets[i].destination(),
            packets[i].size(),
        });
    }

    return Packet::makeCoded(codedId,
                             std::move(relayId),
                             "broadcast",
                             std::move(codedPayload),
                             std::move(constituents));
}

}  // namespace cxor
