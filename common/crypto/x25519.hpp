#pragma once

#include <istream>

#include "key.hpp"

class X25519 : public Key {
   public:
    X25519();
    X25519(const std::string& save_path, const std::string& save_name);
    ~X25519();

   private:
    static std::string hex(const unsigned char* b, size_t n);
};
