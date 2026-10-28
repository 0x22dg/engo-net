#pragma once

#include "../common/logger.hpp"
#include "../common/network/connection.hpp"
#include "identity.hpp"

constexpr uint8_t NONCE_LEN = 32;

class AuthService {
   public:
    AuthService(Logger& logger);

    bool send_hello(Connection& connection, const Identity& identity);
    void handle_server_nonce(Connection& connection, const engo::Packet& packet,
                             const Identity& identity);

   private:
    Logger& logger;
};
