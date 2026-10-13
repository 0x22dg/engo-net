#include "tcp_client.hpp"

TCP_Client::TCP_Client(const std::string& m_addr, uint16_t port, Logger& logger)
    : m_addr(m_addr),
      port(port),
      s_addr(),
      m_socket(std::make_unique<Socket>()),
      logger(logger) {
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = inet_addr(m_addr.c_str());
    s_addr.sin_port = htons(port);

    logger.log(Logger::INFO, "tcp client initialized");
}

bool TCP_Client::connect() {
    return ::connect(m_socket->get(), (struct sockaddr*)&s_addr,
                     sizeof(s_addr)) == 0;
}

Socket& TCP_Client::get_socket() const noexcept { return *m_socket; }
