#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

#include "../common/network/connection.hpp"

struct PubKeyHash {
    size_t operator()(const std::vector<uint8_t>& key) const noexcept {
        size_t hash = 0;
        for (uint8_t u : key)
            hash ^= std::hash<uint8_t>{}(u) + 0x9e3779b9 + (hash << 6) +
                    (hash >> 2);
        return hash;
    }
};

struct UserMeta {
    std::string                 nickname{};
    int                         fd = -1;
    std::vector<uint8_t>        pub_key{};
    std::unique_ptr<Connection> connection = nullptr;

    bool is_empty() const noexcept;
};

class UserRegistry {
   public:
    UserRegistry();
    ~UserRegistry();

    bool                       push_temp(int fd);
    void                       erase_temp(int fd);
    std::optional<Connection*> get_temp(int fd);

    bool move_from_temp(int fd, std::vector<uint8_t> pub_key, std::string name);

    bool push(const std::string& nickname, UserMeta user_meta);

    void erase(int fd);
    void erase(const std::string& nickname);
    void erase(const std::vector<uint8_t>& key);

    UserMeta* find(int fd);
    UserMeta* find(const std::string& nickname);
    UserMeta* find(const std::vector<uint8_t>& key);

   private:
    std::unordered_map<int, Connection> temp_buf;

    std::unordered_map<std::string, UserMeta> src_by_name;

    std::unordered_map<int, UserMeta*>                              src_by_fd;
    std::unordered_map<std::vector<uint8_t>, UserMeta*, PubKeyHash> src_by_key;
};
