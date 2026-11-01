#pragma once

namespace engo {

enum PacketType {
    HELLO = 1,
    AUTH_PROCESS = 2,
    DISCONNECT = 3,
    KEY_EXCHANGE = 4,
    MESSAGE = 5,
    AUTH_ERROR = 6,
    ERROR = 7
};

};
