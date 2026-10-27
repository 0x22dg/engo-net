#include "auth_manager.hpp"

#include <openssl/rand.h>

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

    auto        it = unverified_users.find(fd);
    Connection& connection = *it->connection;

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

std::array<uint8_t, NONCE_LEN> generate_nonce() {
    uint8_t nonce[NONCE_LEN];
    int     rc = RAND_bytes(nonce, sizeof(nonce));
    if (rc != 1)
        throw std::runtime_error(
            "failed to generate the nonce for auth manager");

    return std::to_array(nonce);
}
