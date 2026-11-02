#include "auth_manager.hpp"

#include <openssl/rand.h>

#include "../common/crypto/ed25519.hpp"

AuthManager::AuthManager(Logger& logger) : logger(logger) {}

AuthManager::~AuthManager() {}

void AuthManager::handle_hello_packet(int fd, const engo::Packet& packet) {
    std::vector<uint8_t> pub_key(packet.data.begin(), packet.data.begin() + 32);
    std::string          name(packet.data.begin() + 32, packet.data.end());

    if (!unverified_users.move_from_temp(fd, std::move(pub_key),
                                         std::move(name))) {
        logger.tlog(Logger::ERROR,
                    "failed to move user from temp buffer in auth manager");
        return;
    }

    auto        meta = unverified_users.find(fd);
    Connection& connection = *meta->connection;

    std::array<uint8_t, NONCE_LEN> nonce;
    try {
        nonce = generate_nonce();
    } catch (const std::exception& e) {
        logger.tlog(Logger::ERROR, "failed to generate the nonce");
        return;
    }

    std::vector<uint8_t> v(std::begin(nonce), std::end(nonce));
    if (!connection.send({engo::PacketType::AUTH_PROCESS, v})) {
        logger.tlog(Logger::ERROR, "failed to send the nonce to the user");
        return;
    }

    nonce_buf.emplace(fd, nonce);
}

bool AuthManager::handle_signature(int fd, const engo::Packet& packet) {
    auto                  meta = unverified_users.find(fd);
    std::vector<uint8_t>& pub_key = meta->pub_key;

    auto it = nonce_buf.find(fd);
    if (it == nonce_buf.end()) {
        logger.log(Logger::ERROR, "nonce is not found for this client");
        return false;
    }

    int rc = ED25519::verify(pub_key, {it->second.begin(), it->second.end()},
                             {packet.data.begin(), packet.data.end()});
    if (!rc) {
        logger.log(Logger::ERROR,
                   "failed to verify user signature: wrong signature");

        auto        meta = unverified_users.find(fd);
        Connection& connection = *meta->connection;
        connection.send({engo::PacketType::AUTH_ERROR,
                         "your crypto signature is wrong, auth failed"});
    }

    nonce_buf.erase(it);

    return rc;
}

bool AuthManager::accept_user(int fd, Database& db) {
    auto opt = unverified_users.pop(fd);
    if (!opt) return false;

    int rc = verified_users.push(std::move(*opt));
    if (!rc) return rc;

    return rc;
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

std::array<uint8_t, NONCE_LEN> AuthManager::generate_nonce() {
    uint8_t nonce[NONCE_LEN];
    int     rc = RAND_bytes(nonce, sizeof(nonce));
    if (rc != 1)
        throw std::runtime_error(
            "failed to generate the nonce for auth manager");

    return std::to_array(nonce);
}
