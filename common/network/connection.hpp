#pragma once

#include <arpa/inet.h>
#include <unistd.h>

#include <cstdint>
#include <memory>
#include <queue>
#include <vector>

#include "packet.hpp"
#include "socket.hpp"

#define DEF_BUF_SIZE 1024

class Connection {
   public:
    Connection(int m_socket);
    ~Connection();

    void close() noexcept;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;

    bool send(const engo::Packet& packet);
    bool recv();

    std::unique_ptr<engo::Packet> get_ready_packet();

    const Socket& get_socket() const noexcept;
    bool          is_closed() const noexcept;

   private:
    Socket m_socket;

    std::vector<uint8_t> recv_buf;

    std::queue<std::unique_ptr<engo::Packet>> packet_queue;

    bool closed;

   private:
    void parse_raw(std::vector<uint8_t>& buf);
};
