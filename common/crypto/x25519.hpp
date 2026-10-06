#pragma once

#include "key.hpp"

class X25519 : public Key {
   public:
    X25519();
    ~X25519();

   private:
    static std::string hex(const unsigned char* b, size_t n);
};
