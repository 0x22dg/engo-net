#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
   public:
    Socket();
    Socket(int m_socket);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    bool make_non_blocking();

    int get() const noexcept;

   private:
    int m_socket;
};
