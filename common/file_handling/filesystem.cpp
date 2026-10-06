#include "filesystem.hpp"

#include <filesystem>

using namespace engo;

void filesystem::init() {
    const std::string home = std::string(getenv("HOME"));
    if (home.empty()) throw std::runtime_error("failed to get home dir");

    // ~/.engo-net/
    //  /config
    //  /logs
    //  /db
    std::string main_dir = "/.engo-net/";
    if (!std::filesystem::exists(home + main_dir))
        std::filesystem::create_directory(home + main_dir);
}

std::string filesystem::get_home_path() { return std::string(getenv("HOME")); }

std::string filesystem::get_main_dir(bool with_name) {
    std::string path = "/.engo-net/";
    if (with_name) return get_home_path() + path;
    return path;
}

std::string filesystem::get_identity_dir(bool with_home) {
    return get_main_dir(with_home) + "identity/";
}
