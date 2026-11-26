#include "Node.hpp"
#include "Decoder.hpp"

#include <algorithm>
#include <utility>

namespace cxor {

Node::Node(std::string id) : id_(std::move(id)) {}

void Node::enqueuePacket(Packet packet) {
    rememberPacket(packet);
    outbox_.push(std::move(packet));
}

std::optional<Packet> Node::popOutgoing() {
    if (outbox_.empty()) {
        return std::nullopt;
    }
    Packet next = std::move(outbox_.front());
    outbox_.pop();
    return next;
}

void Node::overhear(const Packet& packet) {
    if (!packet.isCoded()) {
        rememberPacket(packet);
    }
}

bool Node::receive(const Packet& packet) {
    if (!packet.isCoded()) {
        rememberPacket(packet);
        if (packet.destination() == id_) {
            deliveredPackets_.push_back(packet);
            return true;
        }
        return false;
    }

    auto recovered = Decoder::tryDecode(packet, knownPackets_);
    if (!recovered.has_value()) {
        return false;
    }

    rememberPacket(*recovered);
    if (recovered->destination() == id_) {
        deliveredPackets_.push_back(*std::move(recovered));
        return true;
    }
    return false;
}

bool Node::hasKnownPacket(uint64_t packetId) const noexcept {
    return std::any_of(knownPackets_.begin(),
                       knownPackets_.end(),
                       [packetId](const Packet& pkt) { return pkt.id() == packetId; });
}

void Node::rememberPacket(const Packet& packet) {
    if (!hasKnownPacket(packet.id())) {
        knownPackets_.push_back(packet);
    }
}

}  // namespace cxor
