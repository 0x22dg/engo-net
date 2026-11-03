#include "server_state.hpp"

ServerState::ServerState()
    : status(UNKNOWN), addr(""), port(0), mutex(), clients{} {}

ServerState::ServerState(Status status, const std::string& addr, uint16_t port)
    : status(status), addr(addr), port(port) {}

ServerState::Status ServerState::get_status() const noexcept { return status; }

const std::string& ServerState::get_addr() const noexcept { return addr; }

uint16_t ServerState::get_port() const noexcept { return port; }

const char* ServerState::get_status_msg() const noexcept {
    return status_msgs[status];
}

size_t ServerState::get_clients_size() noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    return clients.size();
}

std::vector<std::string> ServerState::get_clients_meta() noexcept {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<std::string> res;
    res.reserve(clients.size());

    for (const auto& c : clients) {
        const ClientMeta& m = c.second;
        std::string       isv = m.is_verified ? "verified" : "unverified";
        res.push_back(m.addr + ":" + std::to_string(m.port) + "; " + isv);
    }

    return std::move(res);
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

void ServerState::push_client(int fd, const std::string& addr, uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex);
    if (addr.empty()) return;

    ClientMeta m = {addr, port, false};
    clients.insert({fd, std::move(m)});
}

std::optional<ServerState::ClientMeta> ServerState::erase_client(int fd) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = clients.find(fd);
    if (it == clients.end()) return std::nullopt;

    ClientMeta meta = std::move(it->second);
    clients.erase(fd);

    return std::move(meta);
}

void ServerState::make_verified(int fd) {
    std::lock_guard<std::mutex> lock(mutex);
    auto                        it = clients.find(fd);
    if (it == clients.end()) return;

    it->second.is_verified = true;
}
