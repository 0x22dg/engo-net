#pragma once

#include <array>
#include <optional>
#include <unordered_map>

#include "../common/logger.hpp"
#include "../common/network/connection.hpp"

constexpr uint8_t NONCE_LEN = 32;

struct VerifiedUser {
    int                  fd;
    std::vector<uint8_t> pub_key;
    Connection           connection;
};

class AuthManager {
   public:
    using UVIterator = std::unordered_map<int, Connection>::iterator;
    using VIterator = std::unordered_map<std::string, VerifiedUser>::iterator;

   public:
    AuthManager();
    ~AuthManager();

    void push_unverified(int fd);
    void delete_unverified(int fd);

    std::optional<UVIterator> find_uv_by_fd(int fd);

    void push_verified(int fd, const std::string& nickname,
                       std::vector<uint8_t> pub_key);
    void delete_verified(int fd);
    void delete_verified(const std::string& nickname);

    std::optional<VIterator> find_v_by_name(const std::string& nickname);
    std::optional<VIterator> find_v_by_fd(int fd);

    std::array<uint8_t, NONCE_LEN> generate_nonce() const;

   private:
    std::unordered_map<int, Connection>           unverified_users;
    std::unordered_map<std::string, VerifiedUser> verified_users;
};
