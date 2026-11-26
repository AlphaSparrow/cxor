#include "Packet.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace cxor {

Packet::Packet(uint64_t id,
               std::string source,
               std::string destination,
               std::vector<uint8_t> payload)
    : id_(id),
      source_(std::move(source)),
      destination_(std::move(destination)),
      payload_(std::move(payload)) {}

Packet::Packet(uint64_t id,
               std::string source,
               std::string destination,
               std::string_view textPayload)
    : id_(id),
      source_(std::move(source)),
      destination_(std::move(destination)),
      payload_(textPayload.begin(), textPayload.end()) {}

Packet::Packet(uint64_t id,
               std::string source,
               std::string destination,
               std::vector<uint8_t> payload,
               std::vector<ConstituentInfo> constituents)
    : id_(id),
      source_(std::move(source)),
      destination_(std::move(destination)),
      payload_(std::move(payload)),
      constituents_(std::move(constituents)) {}

Packet Packet::makeCoded(uint64_t id,
                         std::string source,
                         std::string destination,
                         std::vector<uint8_t> codedPayload,
                         std::vector<ConstituentInfo> constituents) {
    return Packet(id,
                  std::move(source),
                  std::move(destination),
                  std::move(codedPayload),
                  std::move(constituents));
}

std::string Packet::payloadAsString() const {
    return std::string(payload_.begin(), payload_.end());
}

std::string Packet::toHexString() const {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (std::size_t i = 0; i < payload_.size(); ++i) {
        if (i > 0) {
            oss << ' ';
        }
        oss << std::setw(2) << static_cast<unsigned int>(payload_[i]);
    }
    return oss.str();
}

}  // namespace cxor
