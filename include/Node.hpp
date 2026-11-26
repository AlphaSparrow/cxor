#pragma once

#include "Packet.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <queue>
#include <string>
#include <vector>

namespace cxor {

class Node {
public:
    explicit Node(std::string id);

    [[nodiscard]] const std::string& id() const noexcept { return id_; }

    void enqueuePacket(Packet packet);
    [[nodiscard]] bool hasOutgoing() const noexcept { return !outbox_.empty(); }
    [[nodiscard]] std::size_t outboxSize() const noexcept { return outbox_.size(); }
    [[nodiscard]] std::optional<Packet> popOutgoing();

    void overhear(const Packet& packet);
    bool receive(const Packet& packet);

    [[nodiscard]] bool hasKnownPacket(uint64_t packetId) const noexcept;
    [[nodiscard]] const std::vector<Packet>& knownPackets() const noexcept {
        return knownPackets_;
    }
    [[nodiscard]] const std::vector<Packet>& deliveredPackets() const noexcept {
        return deliveredPackets_;
    }

private:
    void rememberPacket(const Packet& packet);

    std::string id_;
    std::queue<Packet> outbox_;
    std::vector<Packet> knownPackets_;
    std::vector<Packet> deliveredPackets_;
};

}  // namespace cxor
