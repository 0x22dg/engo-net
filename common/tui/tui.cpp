#include "tui.hpp"

#include <iostream>

TUI::TUI(const engo::Pair<size_t, size_t>& screen_meta)
    : buf(screen_meta.x, screen_meta.y) {
    sep = std::string(screen_meta.x, '_');

    std::cout << "\033[?25l";
}

TUI::~TUI() { std::cout << "\033[?25h"; }

void TUI::run() {}

void TUI::add_log(const std::string& type, const std::string& message) {
    std::lock_guard lock(log_mutex);
    std::string     msg = type.empty() ? message : type + " -> " + message;
    log_queue.push(msg);
}

const std::string& TUI::get_sep() const noexcept { return sep; }

void TUI::render() {}

void TUI::handle_input() {}

void TUI::make_header(const std::string& title) {
    // TOP SIDE
    buf.put(engo::Pair<size_t, size_t>{0, 1}, title, COLOR_CYAN, COLOR_DEFAULT,
            STYLE_BOLD);
    buf.put(engo::Pair<size_t, size_t>{0, 2}, get_sep(), COLOR_DEFAULT,
            COLOR_DEFAULT, STYLE_BOLD);

    // BOTTOM SIDE
    buf.put(engo::Pair<size_t, size_t>{0, buf.get_wrows() - 3}, get_sep(),
            COLOR_DEFAULT, COLOR_DEFAULT, STYLE_BOLD);
}

void TUI::handle_console_input(Console& console, int key) {
    if (key == InputHandler::BACKSPACE) {
        console.get_input().backspace();
        return;
    }

    if (key >= 32 && key <= 126) {
        console.get_input().put(key);
    }
}

void TUI::process_logs(Console& console, const std::string& author) {
    std::lock_guard lock(log_mutex);

    while (!log_queue.empty()) {
        console.get_message_list().add(author, log_queue.front());
        log_queue.pop();
    }
}
