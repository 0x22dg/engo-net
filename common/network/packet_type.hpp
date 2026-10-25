#pragma once

namespace engo {

enum PacketType {
    AUTH_PROCESS = 1,
    AUTH = 2,
    DISCONNECT = 3,
    KEY_EXCHANGE = 4,
    MESSAGE = 5,
    ERROR = 6
};

};
