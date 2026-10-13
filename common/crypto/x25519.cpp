#include "x25519.hpp"

#include <openssl/evp.h>

#include <memory>

X25519::X25519() : Key{KeyType::X25519, 32} {
    struct PkeyDel {
        void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); }
    };
    using PKEY = std::unique_ptr<EVP_PKEY, PkeyDel>;

    PKEY pkey(EVP_PKEY_Q_keygen(nullptr, nullptr, "X25519"));
    if (!pkey) throw std::runtime_error("failed to init keygen for x25519 key");

    size_t publen = pub.size();
    size_t privlen = priv.size();
    if (EVP_PKEY_get_raw_public_key(pkey.get(), pub.data(), &publen) != 1 ||
        EVP_PKEY_get_raw_private_key(pkey.get(), priv.data(), &privlen) != 1)
        throw std::runtime_error("raw key export failed");
}

X25519::~X25519() { OPENSSL_cleanse(priv.data(), priv.size()); }

std::string X25519::hex(const unsigned char* b, size_t n) {
    static const char* d = "0123456789abcdef";
    std::string        s;
    for (size_t i = 0; i < n; i++) {
        s += d[b[i] >> 4];
        s += d[b[i] & 15];
    }

    return s;
}
