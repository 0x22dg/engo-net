#pragma once

#include "../client/tui/client_tui.hpp"
#include "../client/user.hpp"
#include "../common/command_parser.hpp"
#include "engo_net.hpp"

class EngoClient : public EngoNet {
   public:
    EngoClient(size_t wcols, size_t wrows);
    ~EngoClient();

   private:
    std::unique_ptr<User> user;

    ClientTUI     tui;
    CommandParser command_parser;

    engo::Pair<std::string, std::string> temp_buf;

   private:
    void stop_overall() override;
    void stop_network() override;

    void execute_cmd(const std::string& input);
};
