#include "Decoder.hpp"
#include "Encoder.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace cxor {

namespace {

const Packet* findKnownById(uint64_t id, std::span<const Packet> knownPackets) noexcept {
    for (const auto& pkt : knownPackets) {
        if (pkt.id() == id) {
            return &pkt;
        }
    }
    return nullptr;
}

}  // namespace

bool Decoder::canDecode(const Packet& codedPacket,
                        std::span<const Packet> knownPackets) noexcept {
    if (!codedPacket.isCoded()) {
        return false;
    }

    std::size_t missingCount = 0;
    for (const auto& info : codedPacket.constituents()) {
        if (findKnownById(info.id, knownPackets) == nullptr) {
            ++missingCount;
            if (missingCount > 1) {
                return false;
            }
        }
    }

    return missingCount == 1;
}

std::optional<Packet> Decoder::tryDecode(const Packet& codedPacket,
                                         std::span<const Packet> knownPackets) {
    if (!codedPacket.isCoded()) {
        return std::nullopt;
    }

    const ConstituentInfo* missing = nullptr;
    std::vector<const Packet*> matched;
    matched.reserve(codedPacket.constituents().size());

    for (const auto& info : codedPacket.constituents()) {
        if (const Packet* known = findKnownById(info.id, knownPackets)) {
            matched.push_back(known);
        } else {
            if (missing != nullptr) {
                return std::nullopt;
            }
            missing = &info;
        }
    }

    if (missing == nullptr) {
        return std::nullopt;
    }

    std::vector<uint8_t> recovered = codedPacket.payload();
    for (const Packet* known : matched) {
        recovered = Encoder::xorBytes(recovered, known->payload());
    }
    recovered.resize(missing->originalSize);

    return Packet(missing->id,
                  missing->source,
                  missing->destination,
                  std::move(recovered));
}

Packet Decoder::decode(const Packet& codedPacket,
                       std::span<const Packet> knownPackets) {
    auto result = tryDecode(codedPacket, knownPackets);
    if (!result.has_value()) {
        throw std::invalid_argument(
            "Decoder::decode requires a coded packet with exactly one unknown constituent");
    }
    return *std::move(result);
}

Packet Decoder::decode(const Packet& codedPacket, const Packet& knownPacket) {
    const std::array<Packet, 1> known{knownPacket};
    return decode(codedPacket, known);
}

}  // namespace cxor
