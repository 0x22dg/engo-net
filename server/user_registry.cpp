#include "user_registry.hpp"

bool UserMeta::is_empty() const noexcept {
    return nickname.empty() || fd < 0 || pub_key.empty();
}

UserRegistry::UserRegistry() {}

UserRegistry::~UserRegistry() {
    src_by_key.clear();
    src_by_fd.clear();
    src_by_name.clear();
}

bool UserRegistry::push_temp(int fd) {
    if (fd < 0) return 0;

    temp_buf.emplace(fd, Connection(fd));
    return 1;
}

void UserRegistry::erase_temp(int fd) {
    if (fd < 0) return;

    auto it = temp_buf.find(fd);
    if (it == temp_buf.end()) return;

    temp_buf.erase(it);
}

std::optional<Connection*> UserRegistry::get_temp(int fd) {
    if (fd < 0) return std::nullopt;

    auto it = temp_buf.find(fd);
    if (it == temp_buf.end()) return std::nullopt;

    return &it->second;
}

bool UserRegistry::push(const std::string& nickname, UserMeta user_meta) {
    if (nickname.empty() || user_meta.is_empty()) return 0;

    auto [it, inserted] =
        src_by_name.try_emplace(nickname, std::move(user_meta));

    UserMeta* ptr = &it->second;
    src_by_fd.emplace(user_meta.fd, ptr);
    src_by_key.emplace(user_meta.pub_key, ptr);

    return 1;
}

void UserRegistry::erase(int fd) {
    if (fd < 0) return;

    auto it = src_by_fd.find(fd);
    if (it == src_by_fd.end()) return;

    {
        auto t = src_by_name.find(it->second->nickname);
        src_by_name.erase(t);
    }
    {
        auto t = src_by_key.find(it->second->pub_key);
        src_by_key.erase(t);
    }

    src_by_fd.erase(it);
    return;
}

void UserRegistry::erase(const std::string& nickname) {
    if (nickname.empty()) return;

    auto it = src_by_name.find(nickname);
    if (it == src_by_name.end()) return;

    erase(it->second.fd);
}

void UserRegistry::erase(const std::vector<uint8_t>& key) {
    if (key.empty()) return;

    auto it = src_by_key.find(key);
    if (it == src_by_key.end()) return;

    erase(it->second->fd);
}

UserMeta* UserRegistry::find(int fd) {
    if (fd < 0) return 0;

    auto it = src_by_fd.find(fd);
    if (it == src_by_fd.end()) return nullptr;

    return it->second;
}

UserMeta* UserRegistry::find(const std::string& nickname) {
    if (nickname.empty()) return nullptr;

    auto it = src_by_name.find(nickname);
    if (it == src_by_name.end()) return nullptr;

    return &it->second;
}

UserMeta* UserRegistry::find(const std::vector<uint8_t>& key) {
    if (key.empty()) return nullptr;

    auto it = src_by_key.find(key);
    if (it == src_by_key.end()) return nullptr;

    return it->second;
}
