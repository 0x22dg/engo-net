#include "key.hpp"

#include <fstream>

Key::Key(KeyType type, size_t cap) : type(type), pub(cap), priv(cap) {}

Key::Key(KeyType type, size_t cap, const std::string& save_path,
         const std::string& save_name)
    : type(type), pub(cap), priv(cap) {
    std::string fpath = save_path + "/" + save_name;
    if (!load_key_file(fpath, priv))
        throw std::runtime_error("failed to load private key from file");

    if (!load_key_file(fpath + ".pub", pub))
        throw std::runtime_error("failed to load public key from file");
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

bool Key::load_key_file(const std::string& path, std::vector<uint8_t>& v) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return 0;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buf(size);
    file.read(buf.data(), size);
    std::copy(buf.begin(), buf.end(), v.data());
    return 1;
}
