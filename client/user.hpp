#pragma once

#include <functional>

#include "../common/network/connection.hpp"
#include "auth_service.hpp"
#include "identity.hpp"
#include "tcp_client.hpp"

class User {
   public:
    using MessageCallback = std::function<void(const std::string&)>;

   public:
    User(Identity& identity, const std::string& addr, uint16_t port,
         MessageCallback msg_callback, Logger& logger);

    void run();
    void stop() noexcept;

    bool send(const std::string& message);
    bool send(const std::vector<std::string>& message, size_t arg);

    bool is_enabled() const noexcept;

   private:
    Identity& identity;
    Logger&   logger;

    bool enabled;

    TCP_Client                  tcpc;
    std::unique_ptr<Connection> connection;
    AuthService                 auth_service;
    MessageCallback             msg_callback;
};
