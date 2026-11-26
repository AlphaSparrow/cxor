#pragma once

#include "Packet.hpp"

#include <optional>
#include <span>

namespace cxor {

class Decoder {
public:
    [[nodiscard]] static bool canDecode(const Packet& codedPacket,
                                        std::span<const Packet> knownPackets) noexcept;

    [[nodiscard]] static std::optional<Packet> tryDecode(
        const Packet& codedPacket,
        std::span<const Packet> knownPackets);

    [[nodiscard]] static Packet decode(const Packet& codedPacket,
                                       std::span<const Packet> knownPackets);

    [[nodiscard]] static Packet decode(const Packet& codedPacket,
                                       const Packet& knownPacket);
};

}  // namespace cxor
