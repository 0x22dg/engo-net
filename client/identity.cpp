#include "identity.hpp"

#include "../common/file_handling/filesystem.hpp"

Identity::Identity(const std::string& nickname) : nickname(nickname) {
    ed25519 = std::make_unique<ED25519>();
    x25519 = std::make_unique<X25519>();

    if (!save()) throw std::runtime_error("failed to save the identity keys");
}

Identity::Identity(const std::string&                          nickname,
                   const engo::Pair<std::string, std::string>& edmeta,
                   const engo::Pair<std::string, std::string>& xmeta)
    : nickname(nickname) {
    ed25519 = std::make_unique<ED25519>(edmeta.x, edmeta.y);
    x25519 = std::make_unique<X25519>(xmeta.x, xmeta.y);
}

const std::string& Identity::get_nickname() const noexcept { return nickname; }

ED25519& Identity::get_ed25519() const noexcept { return *ed25519; }

X25519& Identity::get_x25519() const noexcept { return *x25519; }

bool Identity::save() const {
    std::string path = engo::filesystem::get_identity_dir(1);
    path.pop_back();

    bool ed_saved = ed25519->save(path, "ed25519-" + nickname);
    bool x_saved = x25519->save(path, "x25519-" + nickname);

    return ed_saved && x_saved;
}
