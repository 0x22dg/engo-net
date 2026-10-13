#include "engo_server.hpp"

EngoServer::EngoServer(size_t wcols, size_t wrows)
    : EngoNet(),
      tui(
          engo::Pair<size_t, size_t>{wcols, wrows}, state,
          [this](const std::string& cmd) { execute_cmd(cmd); }, get_logger()) {
    get_logger().set_callback(
        [this](const std::string& type, const std::string& msg) {
            tui.add_log(type, msg);
        });

    tui_thread = std::thread([this] {
        while (is_running()) {
            using namespace std::chrono_literals;
            tui.run();
            std::this_thread::sleep_for(16ms);
        }
    });

    get_logger().log(Logger::INFO, "###" + std::string(CONFIG_ENGO_NAME) +
                                       " server side app started");
    get_logger().log(Logger::INFO,
                     "### version: " + std::string(CONFIG_ENGO_VERSION));
    get_logger().log(Logger::INFO,
                     "### source: " + std::string(CONFIG_ENGO_WEB));
}

EngoServer::~EngoServer() { stop_overall(); }

void EngoServer::stop_overall() {
    stop_network();

    if (tui_thread.joinable()) {
        tui_thread.join();
    }

    get_logger().log(Logger::INFO, "engo-net server stopped overall");
}

void EngoServer::stop_network() {
    if (internal_node) {
        internal_node->stop();
        internal_node.reset();
    }

    if (network_thread.joinable()) {
        network_thread.join();
    }

    get_logger().log(Logger::INFO, "engo-net network stopped");
}

void EngoServer::execute_cmd(const std::string& input) {
    Command cmd = command_parser.parse(input);

    switch (cmd.type) {
        case CommandType::EXIT:
            quit();
            break;
        case CommandType::HELP:
            tui.send_help_msg();
            break;
        case CommandType::START:
            if (cmd.args.size() != 2) {
                get_logger().tlog(Logger::ERROR,
                                  "usage: start <ip_addr> <port>");
                break;
            }
            if (!internal_node) {
                internal_node = std::make_unique<InternalNode>(
                    cmd.args[0], static_cast<uint16_t>(std::stoul(cmd.args[1])),
                    state, get_logger());

                network_thread = std::thread([this] { internal_node->run(); });
                get_logger().tlog(Logger::INFO, "server started on ",
                                  cmd.args[0], ":", cmd.args[1]);
            } else {
                get_logger().tlog(Logger::ERROR, "server is already running");
            }
            break;
        case CommandType::STOP:
            stop_network();
            break;
        case CommandType::UNKNOWN:
            get_logger().tlog(Logger::ERROR, "unknown command");
            break;
        default:
            break;
    }
}
