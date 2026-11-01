#include "packet.hpp"

using namespace engo;

Packet::Packet() {}

Packet::Packet(PacketType type, std::vector<uint8_t> payload)
    : header{static_cast<uint16_t>(type),
             static_cast<uint16_t>(payload.size())},
      data(std::move(payload)) {}

Packet::Packet(PacketType type, const std::string& payload)
    : header(static_cast<uint16_t>(type),
             static_cast<uint16_t>(payload.size())),
      data(payload.begin(), payload.end()) {}
