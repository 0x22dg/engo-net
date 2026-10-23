#include "server_state.hpp"

ServerState::ServerState() : status(UNKNOWN), addr(""), port(0), clients{} {}

ServerState::ServerState(Status status, const std::string& addr, uint16_t port)
    : status(status), addr(addr), port(port) {}

ServerState::Status ServerState::get_status() const noexcept { return status; }

const std::string& ServerState::get_addr() const noexcept { return addr; }

uint16_t ServerState::get_port() const noexcept { return port; }

const char* ServerState::get_status_msg() const noexcept {
    return status_msgs[status];
}

const std::vector<engo::Pair<std::string, uint16_t>>& ServerState::get_client()
    const noexcept {
    return clients;
}

void ServerState::set_addr(const std::string& addr) {
    if (addr.empty()) return;
    this->addr = addr;
}

void ServerState::set_port(uint16_t port) { this->port = port; }

bool ServerState::set_status(Status status) {
    if (status < 0 || status > UNKNOWN) return 0;

    this->status = status;
    return 1;
}

void ServerState::push_client(const std::string& addr, uint16_t port) {
    if (addr.empty()) return;

    clients.push_back({addr, port});
}
