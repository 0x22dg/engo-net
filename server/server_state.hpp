#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class ServerState {
   public:
    enum Status { SHUTDOWN, RUNNING, STOPPED, UNKNOWN };

    struct ClientMeta {
        std::string addr;
        uint16_t    port;
        bool        is_verified;
    };

   public:
    ServerState();
    ServerState(Status status, const std::string& addr, uint16_t port);

    Status             get_status() const noexcept;
    const std::string& get_addr() const noexcept;
    uint16_t           get_port() const noexcept;
    const char*        get_status_msg() const noexcept;

    size_t                   get_clients_size() noexcept;
    std::vector<std::string> get_clients_meta() noexcept;

    void set_addr(const std::string& addr);
    void set_port(uint16_t port);
    bool set_status(Status status);

    void push_client(int fd, const std::string& addr, uint16_t port);
    std::optional<ClientMeta> erase_client(int fd);

    void make_verified(int fd);

   private:
    Status      status;
    std::string addr;
    uint16_t    port;

    std::mutex                          mutex;
    std::unordered_map<int, ClientMeta> clients;

    const char* status_msgs[4] = {"SHUTDOWN", "RUNNING", "STOPPED", "UNKNOWN"};
};
