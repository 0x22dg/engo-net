#include "server_tui.hpp"

ServerTUI::ServerTUI(const engo::Pair<size_t, size_t>& screen_meta,
                     ServerState& state, CommandCallback cmd_callback,
                     Logger& logger)
    : TUI(screen_meta),
      state(state),
      cmd_callback(std::move(cmd_callback)),
      logger(logger) {
    buf.clear();

    size_t rect_w = (size_t)screen_meta.x * 0.25;
    rect = std::make_unique<Rect>(
        engo::Pair<size_t, size_t>{0, 3},
        engo::Pair<size_t, size_t>{rect_w, screen_meta.y - 6});

    rect_floor = std::string(rect->get_size().x - 2, '-');

    console = std::make_unique<Console>(
        "admin",
        Rect{{rect->get_vec().x + rect->get_size().x + 1, rect->get_vec().y},
             {screen_meta.x - rect->get_size().x - 1, screen_meta.y - 6}});
}

void ServerTUI::run() {
    handle_input();
    render();
}

void ServerTUI::send_help_msg() {
    std::string msg =
        "exit -> quit from application, help -> send this message, start "
        "<ip_addr> <port> -> start the server, stop -> stop the network side";

    console->get_message_list().add("server", msg);
}

void ServerTUI::render() {
    buf.clear();
    process_logs(*console, "server");
    make_header(CONFIG_ENGO_NAME + std::string(" ") + CONFIG_ENGO_VERSION +
                std::string(" -> server TUI"));

    rect->draw(buf);

    size_t x = rect->get_vec().x + 1;
    size_t y = rect->get_vec().y + 1;

    // Server status >>>
    // 1 section: status, addr, port
    buf.put(engo::Pair<size_t, size_t>{x, y++},
            "state: " + std::string(state.get_status_msg()));
    buf.put(engo::Pair<size_t, size_t>{x, y++}, "address: " + state.get_addr());
    buf.put(engo::Pair<size_t, size_t>{x, y},
            "port: " + std::to_string(state.get_port()));
    y += 2;

    // 2 section: clients, their addrs and ports
    buf.put(engo::Pair<size_t, size_t>{x, y++}, rect_floor, COLOR_DEFAULT,
            COLOR_DEFAULT, STYLE_BOLD);
    buf.put(engo::Pair<size_t, size_t>{x, y++},
            "clients: " + std::to_string(state.get_clients().size()));
    for (size_t i = 0; i < state.get_clients().size(); i++) {
        buf.put(engo::Pair<size_t, size_t>{x, y++},
                state.get_clients()[i].x + ":" +
                    std::to_string(state.get_clients()[i].y) + ": connected");
    }
    y++;
    buf.put(engo::Pair<size_t, size_t>{x, y++}, rect_floor, COLOR_DEFAULT,
            COLOR_DEFAULT, STYLE_BOLD);

    // Console >>
    console->draw(buf);

    buf.render();
}

void ServerTUI::handle_input() {
    int key = input_handler.poll();
    if (key == InputHandler::NONE) return;

    if (key == InputHandler::ENTER1 || key == InputHandler::ENTER2) {
        sumbit_console();
        return;
    }

    handle_console_input(*console, key);
}

void ServerTUI::sumbit_console() {
    std::string cmd = console->sumbit_input();
    if (cmd_callback) {
        cmd_callback(cmd);
    }
}
