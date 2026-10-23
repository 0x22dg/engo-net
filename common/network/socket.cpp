#include "socket.hpp"

#include <fcntl.h>

#include <iostream>

Socket::Socket() : m_socket(-1) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1) throw std::runtime_error("failed to create the socket");
}

Socket::Socket(int m_socket) : m_socket(m_socket) {
    if (m_socket == -1)
        throw std::runtime_error("failed to load the socket fd");
}

Socket::~Socket() {
    if (m_socket >= 0) close(m_socket);
}

Socket::Socket(Socket&& other) noexcept : m_socket(other.m_socket) {
    other.m_socket = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this == &other) return *this;
    if (m_socket >= 0) close(m_socket);

    m_socket = other.m_socket;
    other.m_socket = -1;

    return *this;
}

bool Socket::make_non_blocking() {
    int flags = fcntl(m_socket, F_GETFL, 0);
    if (flags == -1) return 0;

    return fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == 0;
}

int Socket::get() const noexcept { return m_socket; }
