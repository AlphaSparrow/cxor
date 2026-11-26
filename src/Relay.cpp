#include "Relay.hpp"
#include "Encoder.hpp"

#include <utility>

namespace cxor {

Relay::Relay(std::string id, uint64_t startCodedId)
    : id_(std::move(id)), nextCodedId_(startCodedId) {}

void Relay::receive(Packet packet) {
    if (!packet.isCoded()) {
        buffer_.push_back(std::move(packet));
    }
}

std::optional<Packet> Relay::forwardNext(bool codingEnabled) {
    if (buffer_.empty()) {
        return std::nullopt;
    }

    if (codingEnabled && buffer_.size() >= 2) {
        for (std::size_t i = 0; i < buffer_.size(); ++i) {
            for (std::size_t j = i + 1; j < buffer_.size(); ++j) {
                const bool isBidirectionalPair =
                    buffer_[i].source() == buffer_[j].destination() &&
                    buffer_[i].destination() == buffer_[j].source();
                if (isBidirectionalPair) {
                    Packet first = std::move(buffer_[i]);
                    Packet second = std::move(buffer_[j]);
                    buffer_.erase(buffer_.begin() + static_cast<std::ptrdiff_t>(j));
                    buffer_.erase(buffer_.begin() + static_cast<std::ptrdiff_t>(i));

                    ++codedTransmissions_;
                    return Encoder::encode(nextCodedId_++, id_, first, second);
                }
            }
        }
    }

    Packet next = std::move(buffer_.front());
    buffer_.erase(buffer_.begin());
    ++uncodedTransmissions_;
    return next;
}

}  // namespace cxor
