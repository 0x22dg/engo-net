#pragma once

namespace engo {

enum PacketType {
    HELLO = 1,
    AUTH_PROCESS = 2,
    AUTH = 3,
    DISCONNECT = 4,
    KEY_EXCHANGE = 5,
    MESSAGE = 6,
    ERROR = 7
};

};
