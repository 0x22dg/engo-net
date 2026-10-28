#include "engo_client.hpp"

#include <iostream>

EngoClient::EngoClient(size_t wcols, size_t wrows)
    : EngoNet(),
      tui(
          engo::Pair<size_t, size_t>{wcols, wrows},
          [this](const std::string& cmd) { execute_cmd(cmd); }, get_logger()),
      temp_buf{"", ""} {
    // setting the TUI callback log func for engo logger
    get_logger().set_callback(
        [this](const std::string& type, const std::string& msg) {
            tui.add_log(type, msg);
        });

    // creating or loading user indentity
    std::string nickname = get_config().get({"general", "nickname"});
    if (!engo::filesystem::user_keys_existed(nickname)) {
        identity = std::make_unique<Identity>(nickname);
    } else {
        std::string kpath = engo::filesystem::get_identity_dir(1);
        kpath.pop_back();

        engo::Pair<std::string, std::string> edmeta = {kpath,
                                                       "ed25519-" + nickname};
        engo::Pair<std::string, std::string> xmeta = {kpath,
                                                      "x25519-" + nickname};
        identity = std::make_unique<Identity>(nickname, edmeta, xmeta);
    }

    // creating the TUI thread
    tui_thread = std::thread([this] {
        while (is_running()) {
            using namespace std::chrono_literals;
            tui.run();
            std::this_thread::sleep_for(16ms);
        }
    });

    get_logger().log(Logger::INFO, "###" + std::string(CONFIG_ENGO_NAME) +
                                       " client side + app started");
    get_logger().log(Logger::INFO,
                     "### version: " + std::string(CONFIG_ENGO_VERSION));
    get_logger().log(Logger::INFO,
                     "### source: " + std::string(CONFIG_ENGO_WEB));
}

EngoClient::~EngoClient() { stop_overall(); }

void EngoClient::stop_overall() {
    stop_network();

    if (tui_thread.joinable()) {
        tui_thread.join();
    }

    get_logger().log(Logger::INFO, "engo-net client stopped overall");
}

void EngoClient::stop_network() {
    if (user) {
        user->stop();
        user.reset();
    }

    if (network_thread.joinable()) {
        network_thread.join();
    }

    get_logger().log(Logger::INFO, "network stopped");
}

void EngoClient::execute_cmd(const std::string& input) {
    Command cmd = command_parser.parse(input);

    switch (cmd.type) {
        case CommandType::EXIT:
            quit();
            break;
        case CommandType::HELP:
            tui.send_help_msg();
            break;
        case CommandType::STOP:
            stop_network();
            break;
        case CommandType::CONNECT:
            if (cmd.args.size() != 2) {
                get_logger().tlog(Logger::ERROR,
                                  "usage: connect <ip_addr> <port>");
                break;
            }

            if (!user) {
                try {
                    user = std::make_unique<User>(
                        *identity, cmd.args[0],
                        static_cast<uint16_t>(std::stoul(cmd.args[1])),
                        [this](const std::string& msg) {
                            tui.handle_network_msgs(msg);
                        },
                        get_logger());

                    network_thread = std::thread([this] { user->run(); });
                    get_logger().tlog(Logger::INFO, "connected to ",
                                      cmd.args[0], ":", cmd.args[1]);
                } catch (const std::exception& e) {
                    get_logger().log(Logger::ERROR,
                                     "failed to init network user interface: " +
                                         std::string(e.what()));
                    throw;
                }
            } else {
                get_logger().tlog(Logger::ERROR,
                                  "you already connected to the server");
            }

            break;
        case CommandType::ECHO_SEND:
            user->send(cmd.args, 0);
            break;
        case CommandType::UNKNOWN:
            get_logger().tlog(Logger::ERROR, "unknown command");
            break;
        default:
            get_logger().tlog(Logger::ERROR, "wrong command");
            break;
    }
}
