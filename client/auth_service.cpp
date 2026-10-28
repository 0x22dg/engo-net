#include "auth_service.hpp"

AuthService::AuthService(Logger& logger) : logger(logger) {}

bool AuthService::send_hello(Connection& connection, const Identity& identity) {
    std::vector<uint8_t> data = {identity.get_ed25519().get_pub().begin(),
                                 identity.get_ed25519().get_pub().end()};
    data.insert(data.end(), identity.get_nickname().begin(),
                identity.get_nickname().end());

    if (!connection.send({engo::PacketType::HELLO, data})) {
        logger.log(Logger::ERROR,
                   "failed to send the hello packet to the fucking server");
        return 0;
    }

    return 1;
}

void AuthService::handle_server_nonce(Connection&         connection,
                                      const engo::Packet& packet,
                                      const Identity&     identity) {
    if (packet.data.size() != NONCE_LEN) {
        logger.log(
            Logger::ERROR,
            "failed to receive the nonce packet, because packet size is not a "
            "real nonce len");
        return;
    }

    try {
        auto signature = identity.get_ed25519().sign(packet.data);
        if (!connection.send({engo::PacketType::AUTH_PROCESS, signature})) {
            logger.log(Logger::ERROR,
                       "failed to send the signature to the server");
        }
    } catch (const std::exception& e) {
        logger.log(Logger::ERROR, "failed to create the sign: ", e.what());
    }
}
