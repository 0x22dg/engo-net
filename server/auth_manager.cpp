#include "auth_manager.hpp"

#include <openssl/rand.h>

AuthManager::AuthManager() {}

AuthManager::~AuthManager() {
    unverified_users.clear();
    verified_users.clear();
}

void AuthManager::push_unverified(int fd) {
    auto it = unverified_users.find(fd);
    if (it != unverified_users.end()) return;

    unverified_users.emplace(fd, Connection(fd));
}

void AuthManager::delete_unverified(int fd) {
    auto opt = find_uv_by_fd(fd);
    if (!opt) return;

    unverified_users.erase(*opt);
}

std::optional<AuthManager::UVIterator> AuthManager::find_uv_by_fd(int fd) {
    if (fd < 0) return std::nullopt;

    auto it = unverified_users.find(fd);
    if (it != unverified_users.end()) return it;

    return std::nullopt;
}

void AuthManager::push_verified(int fd, const std::string& nickname,
                                std::vector<uint8_t> pub_key) {
    auto uv_opt = find_uv_by_fd(fd);
    if (!uv_opt) return;

    auto v_opt = find_v_by_name(nickname);
    if (v_opt) return;

    UVIterator uv_it = *uv_opt;
    verified_users.emplace(nickname, (VerifiedUser){fd, std::move(pub_key),
                                                    std::move(uv_it->second)});

    unverified_users.erase(uv_it);
}

void AuthManager::delete_verified(int fd) {
    auto opt = find_v_by_fd(fd);
    if (!opt) return;

    verified_users.erase(*opt);
}

void AuthManager::delete_verified(const std::string& nickname) {
    auto opt = find_v_by_name(nickname);
    if (!opt) return;

    verified_users.erase(*opt);
}

std::optional<AuthManager::VIterator> AuthManager::find_v_by_name(
    const std::string& nickname) {
    if (nickname.empty()) return std::nullopt;

    auto it = verified_users.find(nickname);
    if (it != verified_users.end()) return it;

    return std::nullopt;
}

std::optional<AuthManager::VIterator> AuthManager::find_v_by_fd(int fd) {
    if (fd < 0) return std::nullopt;

    auto it =
        std::find_if(verified_users.begin(), verified_users.end(),
                     [fd](const auto& pair) { return pair.second.fd == fd; });

    if (it != verified_users.end()) return it;

    return std::nullopt;
}

std::array<uint8_t, NONCE_LEN> AuthManager::generate_nonce() const {
    uint8_t nonce[NONCE_LEN];
    int     rc = RAND_bytes(nonce, sizeof(nonce));
    if (rc != 1)
        throw std::runtime_error(
            "failed to generate the nonce for auth manager");

    return std::to_array(nonce);
}
