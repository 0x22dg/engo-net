add_library(common-lib
    command_parser.cpp
    logger.cpp
    crypto/ed25519.cpp
    crypto/key.cpp
    crypto/x25519.cpp
    data/database.cpp
    data/db_stmt.cpp
    file_handling/filesystem.cpp
    file_handling/formed_file.cpp
    network/connection.cpp
    network/packet.cpp
    network/socket.cpp
    tui/buffer.cpp
    tui/console.cpp
    tui/input_handler.cpp
    tui/message_list.cpp
    tui/rect.cpp
    tui/text_input.cpp
    tui/tui.cpp
)

target_include_directories(common-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
