#include "engo_net.hpp"

#include <termios.h>
#include <unistd.h>

EngoNet::EngoNet() : run(1) {
    engo::filesystem::init();

    std::string log_file_name = "engo.log";
    logger = std::make_unique<Logger>(engo::filesystem::get_main_dir(1) +
                                      log_file_name);

    std::string cfg_file_name = "config.engo";
    config = std::make_unique<Config>(engo::filesystem::get_main_dir(1) +
                                      cfg_file_name);
    config->load();
}

void EngoNet::quit() noexcept { run = 0; }

bool EngoNet::is_running() const noexcept { return run; }

void EngoNet::stop_overall() {}

void EngoNet::stop_network() {}

Logger& EngoNet::get_logger() const noexcept { return *logger; }

Config& EngoNet::get_config() const noexcept { return *config; }
