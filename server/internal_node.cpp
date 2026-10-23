#include "internal_node.hpp"

#include <arpa/inet.h>
#include <sys/epoll.h>

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
    clients.clear();
    unverified_clients.clear();

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

                    unverified_clients.emplace(client_fd,
                                               Connection(client_fd));
                }
            } else {
                int fd = events[i].data.fd;
                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    client_disconnect(fd);
                    continue;
                }

                // handling socket in the unverified clients holder
                auto it = unverified_clients.find(fd);
                if (it != unverified_clients.end()) {
                    Connection& connection = it->second;
                    if (events[i].events & EPOLLIN) {
                        if (!connection.recv()) {
                            client_disconnect(fd, it);
                            continue;
                        }
                    }

                    while (auto packet = connection.get_ready_packet()) {
                        if (packet->header.type == engo::PacketType::AUTH) {
                        } else if (packet->header.type ==
                                   engo::PacketType::DISCONNECT) {
                            logger.tlog(Logger::INFO,
                                        "disconnecting the unverified user");
                            client_disconnect(fd);
                            break;
                        } else if (packet->header.type ==
                                   engo::PacketType::ERROR) {
                            logger.tlog(
                                Logger::INFO,
                                "error received from the unverified user");
                            client_disconnect(fd);
                            break;
                        }
                    }
                }

                // handling socket in the verified clients holder
                it = clients.find(fd);
                if (it != clients.end()) {
                    Connection& connection = it->second;
                    if (events[i].events & EPOLLIN) {
                        if (!connection.recv()) {
                            client_disconnect(fd, it);
                            continue;
                        }
                    }

                    while (auto packet = connection.get_ready_packet()) {
                        if (packet->header.type == engo::PacketType::MESSAGE) {
                        } else if (packet->header.type ==
                                   engo::PacketType::DISCONNECT) {
                            break;
                        } else if (packet->header.type ==
                                   engo::PacketType::ERROR) {
                            break;
                        }
                    }
                }
            }
        }
    }
}

void InternalNode::stop() noexcept { enabled = 0; }

bool InternalNode::is_enabled() const noexcept { return enabled; }

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

void InternalNode::client_disconnect(int& fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    auto it = clients.find(fd);
    if (it != clients.end()) clients.erase(it);

    it = unverified_clients.find(fd);
    if (it != unverified_clients.end()) unverified_clients.erase(fd);
}
