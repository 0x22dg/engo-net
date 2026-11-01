#include "buffer.hpp"

#include <iostream>

Buffer::Buffer(size_t window_cols, size_t window_rows)
    : window_cols(window_cols),
      window_rows(window_rows),
      size(window_cols * window_rows) {
    actual.resize(size);
    old.resize(size);
}

size_t Buffer::to_index(engo::Pair<size_t, size_t> v) const {
    return v.y * window_cols + v.x;
}

void Buffer::cell_reset(Cell& cell) {
    cell.c = U' ';
    cell.tg = COLOR_DEFAULT;
    cell.bg = COLOR_DEFAULT;
    cell.fl = STYLE_NONE;
}

void Buffer::clear() {
    for (Cell& cell : actual) {
        cell_reset(cell);
    }
}

void Buffer::clear_square(const engo::Pair<size_t, size_t>& v,
                          const engo::Pair<size_t, size_t>& size) {
    if (v.x >= window_cols || v.y >= window_rows) return;

    size_t end_x = std::min(v.x + size.x, window_cols);
    size_t end_y = std::min(v.y + size.y, window_rows);

    for (size_t y = v.y; y < end_y; y++) {
        for (size_t x = v.x; x < end_x; x++) {
            cell_reset(actual[to_index({x, y})]);
        }
    }
}

Cell& Buffer::get_cell(engo::Pair<size_t, size_t> pos) {
    return actual[to_index(pos)];
}

void Buffer::put(engo::Pair<size_t, size_t> v, c32 c, i32 tg, i32 bg, u8 fl) {
    if (v.x >= window_cols || v.y >= window_rows) return;

    size_t width = codepoint_width(c);
    if (width == 0) return;

    if (v.x + width > window_cols) return;

    Cell& cell = get_cell(v);
    cell.c = c;
    cell.tg = tg;
    cell.bg = bg;
    cell.fl = fl;
}

void Buffer::put(engo::Pair<size_t, size_t> v, std::string_view text, i32 tg,
                 i32 bg, u8 fl) {
    size_t offset = 0;

    while (offset < text.size()) {
        c32 c;
        if (!utf8_decode(text, offset, c)) return;

        size_t width = codepoint_width(c);
        if (width == 0) continue;

        if (v.x + width > window_cols) break;
        put(v, c, tg, bg, fl);
        v.x += width;
    }
}

void Buffer::putv(engo::Pair<size_t, size_t> v, std::string_view text, i32 tg,
                  i32 bg, u8 fl) {
    size_t offset = 0;

    while (offset < text.size()) {
        c32 c;
        if (!utf8_decode(text, offset, c)) return;

        size_t width = codepoint_width(c);
        if (width == 0) continue;

        if (v.x + width > window_cols) break;
        put(v, c, tg, bg, fl);
        v.y++;
    }
}

bool Buffer::utf8_decode(std::string_view str, size_t& offset, c32& c) const {
    if (offset >= str.size()) return false;

    const auto byte = [&](size_t index) -> u8 {
        return static_cast<u8>(str[index]);
    };

    u8 b0 = byte(offset);
    if (b0 <= 0x7F) {
        c = b0;
        offset += 1;
        return true;
    }

    if ((b0 & 0xE0) == 0xC0) {
        if (offset + 1 >= str.size()) return false;
        u8 b1 = byte(offset + 1);

        if ((b1 & 0xC0) != 0x80) return false;
        c = ((b0 & 0x1F) << 6) | (b1 & 0x3F);

        offset += 2;
        return true;
    }

    if ((b0 & 0xF0) == 0xE0) {
        if (offset + 2 >= str.size()) return false;
        u8 b1 = byte(offset + 1);
        u8 b2 = byte(offset + 2);

        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return 0;
        c = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);

        offset += 3;
        return true;
    }

    if ((b0 & 0xF8) == 0xF0) {
        if (offset + 3 >= str.size()) return false;

        u8 b1 = byte(offset + 1);
        u8 b2 = byte(offset + 2);
        u8 b3 = byte(offset + 3);

        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            return false;
        }

        c = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) |
            (b3 & 0x3F);

        offset += 4;
        return true;
    }

    return false;
}

void Buffer::utf8_encode(c32 c, std::string& output) const {
    if (c <= 0x7F) {
        output.push_back(static_cast<char>(c));
    } else if (c <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (c >> 6)));
        output.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else if (c <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (c >> 12)));
        output.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else if (c <= 0x10FFFF) {
        output.push_back(static_cast<char>(0xF0 | (c >> 18)));
        output.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
}

size_t Buffer::codepoint_width(c32 cp) const {
    if (cp < 0x20) return 0;

    if (cp >= 0x7F && cp <= 0x9F) return 0;

    if ((cp >= 0x0300 && cp <= 0x036F) || (cp >= 0x1AB0 && cp <= 0x1AFF) ||
        (cp >= 0x1DC0 && cp <= 0x1DFF) || (cp >= 0x20D0 && cp <= 0x20FF) ||
        (cp >= 0xFE20 && cp <= 0xFE2F)) {
        return 0;
    }

    if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2329 && cp <= 0x232A) ||
        (cp >= 0x2E80 && cp <= 0xA4CF) || (cp >= 0xAC00 && cp <= 0xD7A3) ||
        (cp >= 0xF900 && cp <= 0xFAFF) || (cp >= 0xFE10 && cp <= 0xFE19) ||
        (cp >= 0xFE30 && cp <= 0xFE6F) || (cp >= 0xFF00 && cp <= 0xFF60) ||
        (cp >= 0xFFE0 && cp <= 0xFFE6) || (cp >= 0x1F300 && cp <= 0x1FAFF)) {
        return 2;
    }

    return 1;
}

void Buffer::cell_cmpl(Cell& cell, i32& c_tg, i32& c_bg, u8& c_fl) {
    if (cell.tg != c_tg || cell.bg != c_bg || cell.fl != c_fl) {
        std::cout << "\033[0m";

        if (cell.fl & STYLE_BOLD) {
            std::cout << "\033[1m";
        }

        if (cell.fl & STYLE_UNDERLINE) {
            std::cout << "\033[4m";
        }

        if (cell.tg != COLOR_DEFAULT) {
            std::cout << "\033[" << (30 + cell.tg) << "m";
        }

        if (cell.bg != COLOR_DEFAULT) {
            std::cout << "\033[" << (40 + cell.bg) << "m";
        }

        c_tg = cell.tg;
        c_bg = cell.bg;
        c_fl = cell.fl;
    }
}

void Buffer::render_cell(const Cell& cell) {
    std::string utf8;
    utf8_encode(cell.c, utf8);

    std::cout << utf8;
}

void Buffer::render() {
    i32 current_tg = COLOR_DEFAULT;
    i32 current_bg = COLOR_DEFAULT;
    u8  current_fl = STYLE_NONE;

    for (size_t y = 0; y < window_rows; ++y) {
        for (size_t x = 0; x < window_cols; ++x) {
            engo::Pair<size_t, size_t> pos{.x = x, .y = y};

            Cell& current = actual[to_index(pos)];
            Cell& previous = old[to_index(pos)];

            if (current == previous) continue;

            std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";

            cell_cmpl(current, current_tg, current_bg, current_fl);

            render_cell(current);
        }
    }

    std::cout << "\033[0m";
    old = actual;
    std::cout.flush();
}

std::vector<std::string> Buffer::wrap(std::string_view text,
                                      size_t           max_width) const {
    std::vector<std::string> lines;
    if (max_width == 0) return lines;

    std::string current;
    size_t      current_width = 0;
    size_t      offset = 0;

    while (offset < text.size()) {
        size_t char_offset = offset;

        c32 c;
        if (!utf8_decode(text, offset, c)) break;

        size_t width = codepoint_width(c);
        if (width == 0) continue;
        if (current_width + width > max_width) {
            if (!current.empty()) {
                lines.push_back(std::move(current));
                current.clear();
            }

            current_width = 0;

            if (width > max_width) continue;
        }

        current.append(text.substr(char_offset, offset - char_offset));
        current_width += width;
    }

    if (!current.empty()) {
        lines.push_back(std::move(current));
    }

    return lines;
}

size_t Buffer::get_wcols() const noexcept { return window_cols; }

size_t Buffer::get_wrows() const noexcept { return window_rows; }
