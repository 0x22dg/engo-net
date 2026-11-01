#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "packet_type.hpp"

namespace engo {

struct PacketHeader {
    uint16_t type;
    uint16_t payload_size;
};

struct Packet {
    Packet(PacketType type, std::vector<uint8_t> payload);
    Packet(PacketType type, const std::string& payload);
    Packet();

    PacketHeader         header;
    std::vector<uint8_t> data;
};

};  // namespace engo
