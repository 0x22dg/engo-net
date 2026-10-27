#pragma once

#include <sys/epoll.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/logger.hpp"
#include "../common/network/connection.hpp"
#include "auth_manager.hpp"
#include "server_state.hpp"
#include "tcp_server.hpp"

#define MAX_EVENTS 64
#define EPOLL_TIMEOUT 100

class InternalNode {
   public:
    InternalNode(const std::string& m_addr, uint16_t port, ServerState& state,
                 Logger& logger);
    ~InternalNode();

    void run();
    void stop() noexcept;

    bool is_enabled() const noexcept;

   private:
    ServerState& state;
    Logger&      logger;

    std::unique_ptr<TCP_Server> tcps;

    AuthManager auth_manager;

    int                      epoll_fd;
    std::vector<epoll_event> events;

    bool enabled;

   private:
    bool handle_unverified(int fd, size_t idx);
    bool handle_verified(int fd, size_t idx);

    int  receive(ServerState& state);
    void disconnect_temp(int fd);
    void disconnect_v(int fd);
};
