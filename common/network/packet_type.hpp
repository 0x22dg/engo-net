#pragma once

namespace engo {

enum PacketType {
    AUTH = 1,
    DISCONNECT = 2,
    KEY_EXCHANGE = 3,
    MESSAGE = 4,
    ERROR = 5
};

};
