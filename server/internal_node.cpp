#include "internal_node.hpp"

#include <arpa/inet.h>
#include <sys/epoll.h>

#include <cstdint>

InternalNode::InternalNode(const std::string& m_addr, uint16_t port,
                           ServerState& state, Logger& logger)
    : tcps(std::make_unique<TCP_Server>(m_addr, port, logger)),
      state(state),
      logger(logger),
      epoll_fd(-1),
      events(),
      enabled(1) {
    std::string msg = m_addr + "/" + std::to_string(port);
    if (!tcps->bind())
        throw std::runtime_error("failed to bind the internal node: " + msg);

    if (!tcps->listen())
        throw std::runtime_error("failed to listen in the internal node: " +
                                 msg);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
        throw std::runtime_error("failed to create epoll: " + msg);

    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.fd = tcps->get_socket().get();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcps->get_socket().get(), &event) ==
        -1)
        throw std::runtime_error("failed to epoll ctl: " + msg);

    events.resize(MAX_EVENTS);

    state.set_status(ServerState::RUNNING);
    state.set_addr(m_addr);
    state.set_port(port);

    logger.log(Logger::INFO, "internal node started on: " + msg);
}

InternalNode::~InternalNode() {
    if (epoll_fd >= 0) {
        close(epoll_fd);
    }

    state.set_status(ServerState::STOPPED);
}

void InternalNode::run() {
    while (enabled) {
        int num_events =
            epoll_wait(epoll_fd, events.data(), MAX_EVENTS, EPOLL_TIMEOUT);
        if (num_events == -1) {
            enabled = 0;
            logger.log(Logger::ERROR,
                       "failed to wait for events in the internal node");
            break;
        }

        if (num_events == 0) continue;

        for (size_t i = 0; i < num_events; i++) {
            if (events[i].data.fd == tcps->get_socket().get()) {
                while (1) {
                    int client_fd = receive(state);
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        if (errno == EINTR) continue;

                        logger.log(Logger::ERROR,
                                   "failed to accept the client connection");
                        break;
                    }

                    epoll_event event{};
                    event.events = EPOLLIN | EPOLLRDHUP;
                    event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) ==
                        -1) {
                        close(client_fd);
                        logger.log(Logger::ERROR,
                                   "failed to add the client fd to the epoll");
                        continue;
                    }

                    auth_manager.get_unverified_rgs().push_temp(client_fd);
                }
            } else {
                int fd = events[i].data.fd;
                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    auth_manager.disconnect_uv(fd);
                    auth_manager.disconnect_v(fd);
                    continue;
                }

                if (!handle_unverified(fd, i)) continue;
                if (!handle_verified(fd, i)) continue;
            }
        }
    }
}

void InternalNode::stop() noexcept { enabled = 0; }

bool InternalNode::is_enabled() const noexcept { return enabled; }

bool InternalNode::handle_unverified(int fd, size_t idx) {
    auto opt = auth_manager.get_unverified_rgs().get_temp(fd);
    if (!opt) return 1;

    Connection* connection = *opt;

    if (events[idx].events & EPOLLIN) {
        if (!connection->recv()) {
            disconnect_temp(fd);
            return 0;
        }
    }

    while (auto packet = connection->get_ready_packet()) {
        switch (packet->header.type) {
            case engo::PacketType::HELLO: {
                std::vector<uint8_t> pub_key(packet->data.begin(),
                                             packet->data.begin() + 32);
                std::string name(packet->data.begin() + 32, packet->data.end());
                if (!auth_manager.get_unverified_rgs().move_from_temp(
                        fd, std::move(pub_key), std::move(name))) {
                    logger.tlog(
                        Logger::ERROR,
                        "failed to move user from temp buffer in auth manager");
                    break;
                }
            } break;
            case engo::PacketType::AUTH_PROCESS:
                break;
            case engo::PacketType::AUTH:
                break;
            case engo::PacketType::DISCONNECT:
                logger.tlog(Logger::INFO, "disconnecting the unverified user");
                disconnect_temp(fd);
                break;
            case engo::PacketType::ERROR:
                logger.tlog(Logger::INFO,
                            "error received from the unverified user");
                disconnect_temp(fd);
                break;
            default:
                disconnect_temp(fd);
                break;
        }
    }

    return 1;
}

bool InternalNode::handle_verified(int fd, size_t idx) {
    UserMeta* meta = auth_manager.get_verified_rgs().find(fd);

    Connection* connection = meta->connection.get();
    if (events[idx].events & EPOLLIN) {
        if (!connection->recv()) {
            disconnect_v(fd);
            return 0;
        }
    }

    while (auto packet = connection->get_ready_packet()) {
        switch (packet->header.type) {
            case engo::PacketType::MESSAGE:
                break;
            case engo::PacketType::DISCONNECT:
                logger.tlog(Logger::INFO, "verified user disconnected");
                disconnect_v(fd);
                break;
            case engo::PacketType::ERROR:
                logger.tlog(Logger::INFO,
                            "error received from the unverfied user");
                disconnect_v(fd);
                break;
            default:
                disconnect_v(fd);
                break;
        }
    }

    return 1;
}

int InternalNode::receive(ServerState& state) {
    sockaddr_in client_addr{};
    socklen_t   client_addr_len = sizeof(client_addr);
    int         client_fd =
        accept4(tcps->get_socket().get(), (struct sockaddr*)&client_addr,
                &client_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (client_fd != -1) {
        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, sizeof(ipbuf));

        std::string ip(ipbuf);
        uint16_t    port = ntohs(client_addr.sin_port);

        state.push_client(ip, port);
    }

    return client_fd;
}

void InternalNode::disconnect_temp(int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    auth_manager.get_unverified_rgs().erase_temp(fd);
}

void InternalNode::disconnect_v(int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    auth_manager.get_verified_rgs().erase(fd);
}
