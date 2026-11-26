#pragma once

#include "Packet.hpp"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace cxor {

class Encoder {
public:
    [[nodiscard]] static std::vector<uint8_t> xorBytes(std::span<const uint8_t> a,
                                                       std::span<const uint8_t> b);

    [[nodiscard]] static Packet encode(uint64_t codedId,
                                       std::string relayId,
                                       const Packet& a,
                                       const Packet& b);

    [[nodiscard]] static Packet encodeMany(uint64_t codedId,
                                           std::string relayId,
                                           std::span<const Packet> packets);
};

}  // namespace cxor
