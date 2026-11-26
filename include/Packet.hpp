#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cxor {

struct ConstituentInfo {
    uint64_t id{};
    std::string source;
    std::string destination;
    std::size_t originalSize{};

    bool operator==(const ConstituentInfo&) const = default;
};

class Packet {
public:
    Packet() = default;

    Packet(uint64_t id,
           std::string source,
           std::string destination,
           std::vector<uint8_t> payload);

    Packet(uint64_t id,
           std::string source,
           std::string destination,
           std::string_view textPayload);

    static Packet makeCoded(uint64_t id,
                            std::string source,
                            std::string destination,
                            std::vector<uint8_t> codedPayload,
                            std::vector<ConstituentInfo> constituents);

    [[nodiscard]] uint64_t id() const noexcept { return id_; }
    [[nodiscard]] const std::string& source() const noexcept { return source_; }
    [[nodiscard]] const std::string& destination() const noexcept { return destination_; }
    [[nodiscard]] const std::vector<uint8_t>& payload() const noexcept { return payload_; }
    [[nodiscard]] std::size_t size() const noexcept { return payload_.size(); }

    [[nodiscard]] bool isCoded() const noexcept { return !constituents_.empty(); }
    [[nodiscard]] const std::vector<ConstituentInfo>& constituents() const noexcept {
        return constituents_;
    }

    [[nodiscard]] std::string payloadAsString() const;
    [[nodiscard]] std::string toHexString() const;

    bool operator==(const Packet&) const = default;

private:
    Packet(uint64_t id,
           std::string source,
           std::string destination,
           std::vector<uint8_t> payload,
           std::vector<ConstituentInfo> constituents);

    uint64_t id_{};
    std::string source_;
    std::string destination_;
    std::vector<uint8_t> payload_;
    std::vector<ConstituentInfo> constituents_;
};

}  // namespace cxor
