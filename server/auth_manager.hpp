#pragma once

#include <array>
#include <cstdint>

#include "../common/logger.hpp"
#include "../common/network/packet.hpp"
#include "user_registry.hpp"

constexpr uint8_t NONCE_LEN = 32;

class AuthManager {
   public:
    AuthManager(Logger& logger);
    ~AuthManager();

    void handle_hello_packet(int fd, const engo::Packet& packet);

    void disconnect_uv(int fd);
    void disconnect_v(int fd);

    UserRegistry& get_unverified_rgs() noexcept;
    UserRegistry& get_verified_rgs() noexcept;

   private:
    Logger& logger;

    UserRegistry unverified_users;
    UserRegistry verified_users;

    std::unordered_map<int, std::array<uint8_t, NONCE_LEN>> nonce_buf;

   private:
    static std::array<uint8_t, NONCE_LEN> generate_nonce();
};
