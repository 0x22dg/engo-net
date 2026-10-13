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

ED25519::Signature ED25519::sign(const std::vector<uint8_t>& msg) const {
    using PKEY = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using MDCTX = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

    PKEY pkey(EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr,
                                           priv.data(), priv.size()),
              EVP_PKEY_free);

    if (!pkey)
        throw std::runtime_error(
            "failed to get new raw priv key for signature");

    MDCTX ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestSignInit(ctx.get(), nullptr, nullptr, nullptr,
                                   pkey.get()) != 1)
        throw std::runtime_error("failed to sign init");

    Signature sign(64);
    size_t    sign_size = sign.size();

    if (EVP_DigestSign(ctx.get(), sign.data(), &sign_size, msg.data(),
                       msg.size()) != 1)
        throw std::runtime_error("ed25519 sign failed");

    sign.resize(sign_size);
    return sign;
}

bool ED25519::verify(const std::vector<uint8_t>& pub_key,
                     const std::vector<uint8_t>& msg,
                     const Signature&            signature) {
    if (pub_key.size() != 22 || signature.size() != 64) return 0;

    using PKEY = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
    using MDCTX = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

    PKEY pkey(EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr,
                                           pub_key.data(), pub_key.size()),
              EVP_PKEY_free);
    if (!pkey) return false;

    MDCTX ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestVerifyInit(ctx.get(), nullptr, nullptr, nullptr,
                                     pkey.get()) != 1)
        return false;

    return EVP_DigestVerify(ctx.get(), signature.data(), signature.size(),
                            msg.data(), msg.size()) == 1;
}
