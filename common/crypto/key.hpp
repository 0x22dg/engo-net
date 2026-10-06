#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "key_type.hpp"

class Key {
   public:
    Key(KeyType type, size_t cap);

    bool save(const std::string& save_path, const std::string& save_name) const;

    KeyType get_type() const noexcept;

    const std::vector<uint8_t>& get_pub() const noexcept;
    const std::vector<uint8_t>& get_priv() const noexcept;

   private:
    KeyType type;

    std::vector<uint8_t> pub;
    std::vector<uint8_t> priv;

   private:
    bool save_key_file(const std::string&          path,
                       const std::vector<uint8_t>& v) const;
};
