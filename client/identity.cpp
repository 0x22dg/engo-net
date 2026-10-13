#include "identity.hpp"

#include "../common/file_handling/filesystem.hpp"

Identity::Identity(const std::string& nickname)
    : nickname(nickname), need_to_save(1) {
    ed25519 = std::make_unique<ED25519>();
    x25519 = std::make_unique<X25519>();
}

Identity::Identity(const std::string&                          nickname,
                   const engo::Pair<std::string, std::string>& edmeta,
                   const engo::Pair<std::string, std::string>& xmeta)
    : nickname(nickname), need_to_save(0) {
    ed25519 = std::make_unique<ED25519>(edmeta.x, edmeta.y);
    x25519 = std::make_unique<X25519>(xmeta.x, xmeta.y);
}

Identity::~Identity() {
    if (need_to_save) {
        std::string path = engo::filesystem::get_identity_dir(1);
        path.pop_back();

        ed25519->save(path, "ed25519-" + nickname);
        x25519->save(path, "x25519-" + nickname);
    }
}

const std::string& Identity::get_nickname() const noexcept { return nickname; }

ED25519& Identity::get_ed25519() const noexcept { return *ed25519; }

X25519& Identity::get_x25519() const noexcept { return *x25519; }
