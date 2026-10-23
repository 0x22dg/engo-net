#include "connection.hpp"

#include <sys/socket.h>

#include <cerrno>
#include <cstdint>
#include <iostream>

Connection::Connection(int m_socket) : m_socket(m_socket), closed(0) {
    recv_buf.reserve(DEF_BUF_SIZE);
}

Connection::~Connection() { close(); }

void Connection::close() noexcept { closed = 1; }

Connection::Connection(Connection&& other) noexcept
    : m_socket(std::move(other.m_socket)),
      recv_buf(std::move(other.recv_buf)),
      closed(other.closed) {
    other.m_socket = -1;
    other.closed = 1;
}

Connection& Connection::operator=(Connection&& other) noexcept {
    if (this == &other) return *this;

    close();

    m_socket = std::move(other.m_socket);
    recv_buf = std::move(other.recv_buf);
    closed = other.closed;

    other.m_socket = -1;
    other.closed = true;

    return *this;
}

bool Connection::send(const engo::Packet& packet) {
    engo::PacketHeader header{htons(packet.header.type),
                              htons(packet.header.payload_size)};

    size_t total_size = sizeof(engo::PacketHeader) + packet.data.size();

    std::vector<uint8_t> raw(total_size);
    std::copy(
        reinterpret_cast<const uint8_t*>(&header),
        reinterpret_cast<const uint8_t*>(&header) + sizeof(engo::PacketHeader),
        raw.begin());

    std::copy(packet.data.begin(), packet.data.end(),
              raw.begin() + sizeof(engo::PacketHeader));

    size_t sent = 0;

    while (sent < total_size) {
        ssize_t val =
            ::send(m_socket.get(), raw.data() + sent, raw.size() - sent, 0);

        if (val < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return 0;
        }

        sent += val;
    }

    return 1;
}

bool Connection::recv() {
    char temp[4096];
    while (1) {
        ssize_t val = ::recv(m_socket.get(), temp, sizeof(temp), 0);
        if (val > 0) {
            recv_buf.insert(recv_buf.end(), temp, temp + val);
            parse_raw(recv_buf);
            continue;
        }

        if (val == 0) {
            close();
            return 0;
        }

        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 1;

        close();
        return 0;
    }
}

std::unique_ptr<engo::Packet> Connection::get_ready_packet() {
    if (packet_queue.empty()) return nullptr;
    std::unique_ptr<engo::Packet> p = std::move(packet_queue.front());
    packet_queue.pop();
    return p;
}

const Socket& Connection::get_socket() const noexcept { return m_socket; }

bool Connection::is_closed() const noexcept { return closed; }

void Connection::parse_raw(std::vector<uint8_t>& buf) {
    while (buf.size() >= sizeof(engo::PacketHeader)) {
        engo::PacketHeader* header =
            reinterpret_cast<engo::PacketHeader*>(buf.data());

        uint16_t payload_size = ntohs(header->payload_size);
        size_t   packet_size = payload_size + sizeof(engo::PacketHeader);

        if (buf.size() < packet_size) break;

        std::unique_ptr<engo::Packet> fresh_packet =
            std::make_unique<engo::Packet>();

        fresh_packet->header.type = ntohs(header->type);
        fresh_packet->header.payload_size = payload_size;

        fresh_packet->data.assign(buf.begin() + sizeof(engo::PacketHeader),
                                  buf.begin() + packet_size);

        packet_queue.push(std::move(fresh_packet));

        buf.erase(buf.begin(), buf.begin() + packet_size);
    }
}
