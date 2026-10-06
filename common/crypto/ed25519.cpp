#include "ed25519.hpp"

#include <openssl/evp.h>

#include <memory>

ED25519::ED25519() : Key(KeyType::ED25519, 32) {
    using CTX = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
    using PKEY = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

    CTX ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr), EVP_PKEY_CTX_free);
    EVP_PKEY* raw = nullptr;
    if (!ctx || EVP_PKEY_keygen_init(ctx.get()) <= 0 ||
        EVP_PKEY_keygen(ctx.get(), &raw) <= 0)
        throw std::runtime_error("failed to init ed25519 keygen");

    PKEY pkey(raw, EVP_PKEY_free);

    size_t priv_len = 0, pub_len = 0;
    if (EVP_PKEY_get_raw_private_key(pkey.get(), nullptr, &priv_len) <= 0 ||
        EVP_PKEY_get_raw_public_key(pkey.get(), nullptr, &pub_len) <= 0)
        throw std::runtime_error("failed to get private or public ed25519 key");

    if (EVP_PKEY_get_raw_private_key(pkey.get(), priv.data(), &priv_len) <= 0 ||
        EVP_PKEY_get_raw_public_key(pkey.get(), pub.data(), &pub_len) <= 0)
        throw std::runtime_error("failed to get private or public ed25519 key");
}

ED25519::~ED25519() { OPENSSL_cleanse(priv.data(), priv.size()); }
