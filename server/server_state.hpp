#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/utils/pair.hpp"

class ServerState {
   public:
    enum Status { SHUTDOWN, RUNNING, STOPPED, UNKNOWN };

   public:
    ServerState();
    ServerState(Status status, const std::string& addr, uint16_t port);

    Status             get_status() const noexcept;
    const std::string& get_addr() const noexcept;
    uint16_t           get_port() const noexcept;
    const char*        get_status_msg() const noexcept;
    const std::vector<engo::Pair<std::string, uint16_t>>& get_clients()
        const noexcept;

    void set_addr(const std::string& addr);
    void set_port(uint16_t port);
    bool set_status(Status status);
    void push_client(const std::string& addr, uint16_t port);

   private:
    Status      status;
    std::string addr;
    uint16_t    port;

    std::vector<engo::Pair<std::string, uint16_t>> clients;

    const char* status_msgs[4] = {"SHUTDOWN", "RUNNING", "STOPPED", "UNKNOWN"};
};
