#pragma once

#include <string>

namespace engo {

namespace filesystem {

void init();

std::string get_home_path();
std::string get_main_dir(bool with_home);
std::string get_identity_dir(bool with_home);

bool are_user_keys_exist(const std::string& nickname);

};  // namespace filesystem

};  // namespace engo
