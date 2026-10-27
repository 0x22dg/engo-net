#include "auth_manager.hpp"

#include <openssl/rand.h>

AuthManager::AuthManager() {}

AuthManager::~AuthManager() {}

std::array<uint8_t, NONCE_LEN> AuthManager::generate_nonce() const {
    uint8_t nonce[NONCE_LEN];
    int     rc = RAND_bytes(nonce, sizeof(nonce));
    if (rc != 1)
        throw std::runtime_error(
            "failed to generate the nonce for auth manager");

    return std::to_array(nonce);
}

void AuthManager::disconnect_uv(int fd) {
    if (fd < 0) return;

    unverified_users.erase_temp(fd);
    unverified_users.erase(fd);
}

void AuthManager::disconnect_v(int fd) {
    if (fd < 0) return;

    verified_users.erase_temp(fd);
    verified_users.erase(fd);
}

UserRegistry& AuthManager::get_unverified_rgs() noexcept {
    return unverified_users;
}

UserRegistry& AuthManager::get_verified_rgs() noexcept {
    return verified_users;
}
