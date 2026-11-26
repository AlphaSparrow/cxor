#pragma once

#include "Packet.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cxor {

class Relay {
public:
    explicit Relay(std::string id, uint64_t startCodedId = 1000);

    [[nodiscard]] const std::string& id() const noexcept { return id_; }

    void receive(Packet packet);

    [[nodiscard]] bool hasPending() const noexcept { return !buffer_.empty(); }
    [[nodiscard]] std::size_t pendingCount() const noexcept { return buffer_.size(); }

    [[nodiscard]] std::optional<Packet> forwardNext(bool codingEnabled = true);

    [[nodiscard]] uint64_t codedTransmissions() const noexcept {
        return codedTransmissions_;
    }
    [[nodiscard]] uint64_t uncodedTransmissions() const noexcept {
        return uncodedTransmissions_;
    }
    [[nodiscard]] uint64_t totalTransmissions() const noexcept {
        return codedTransmissions_ + uncodedTransmissions_;
    }

private:
    std::string id_;
    uint64_t nextCodedId_;
    std::vector<Packet> buffer_;
    uint64_t codedTransmissions_{0};
    uint64_t uncodedTransmissions_{0};
};

}  // namespace cxor
