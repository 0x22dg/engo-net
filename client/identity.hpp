#pragma once

#include <memory>

#include "../common/crypto/ed25519.hpp"
#include "../common/crypto/x25519.hpp"
#include "../common/utils/pair.hpp"

class Identity {
   public:
    Identity(const std::string& nickname);
    Identity(const std::string&                          nickname,
             const engo::Pair<std::string, std::string>& edmeta,
             const engo::Pair<std::string, std::string>& xmeta);
    ~Identity();

    const std::string& get_nickname() const noexcept;
    ED25519&           get_ed25519() const noexcept;
    X25519&            get_x25519() const noexcept;

   private:
    bool need_to_save;

    std::string nickname;

    std::unique_ptr<ED25519> ed25519;
    std::unique_ptr<X25519>  x25519;
};
