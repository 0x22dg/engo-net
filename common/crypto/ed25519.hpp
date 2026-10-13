#pragma once

#include "key.hpp"

class ED25519 : public Key {
   public:
    using Signature = std::vector<uint8_t>;

   public:
    ED25519();
    ~ED25519();

    Signature sign(const std::vector<uint8_t>& msg) const;

    static bool verify(const std::vector<uint8_t>& pub_key,
                       const std::vector<uint8_t>& msg,
                       const Signature&            signature);
};
