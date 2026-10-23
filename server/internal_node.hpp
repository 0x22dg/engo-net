#pragma once

#include <sys/epoll.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/logger.hpp"
#include "../common/network/connection.hpp"
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

    int                      epoll_fd;
    std::vector<epoll_event> events;

    std::unordered_map<int, Connection> clients;
    std::unordered_map<int, Connection> unverified_clients;

    bool enabled;

   private:
    int  receive(ServerState& state);
    void client_disconnect(int& fd);

    template <typename T>
    void client_disconnect(int& fd, T it) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        clients.erase(it);
    }
};
