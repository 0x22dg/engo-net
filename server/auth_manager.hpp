#pragma once

#include <array>
#include <cstdint>

#include "user_registry.hpp"

constexpr uint8_t NONCE_LEN = 32;

class AuthManager {
   public:
    AuthManager();
    ~AuthManager();

    std::array<uint8_t, NONCE_LEN> generate_nonce() const;

    void disconnect_uv(int fd);
    void disconnect_v(int fd);

    UserRegistry& get_unverified_rgs() noexcept;
    UserRegistry& get_verified_rgs() noexcept;

   private:
    UserRegistry unverified_users;
    UserRegistry verified_users;
};
