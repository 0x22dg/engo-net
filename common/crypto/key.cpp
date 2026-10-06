#include "key.hpp"

#include <fstream>

Key::Key(KeyType type, size_t cap) : type(type), pub{}, priv{} {
    pub.reserve(cap);
    priv.reserve(cap);
}

bool Key::save(const std::string& save_path,
               const std::string& save_name) const {
    return save_key_file(save_path + "/" + save_name, priv) &&
           save_key_file(save_path + "/" + save_name + ".pub", pub);
}

KeyType Key::get_type() const noexcept { return type; }

const std::vector<uint8_t>& Key::get_pub() const noexcept { return pub; }

const std::vector<uint8_t>& Key::get_priv() const noexcept { return priv; }

bool Key::save_key_file(const std::string&          path,
                        const std::vector<uint8_t>& v) const {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file.is_open()) return 0;

    file.write(reinterpret_cast<const char*>(v.data()), v.size());
    return file.good();
}
