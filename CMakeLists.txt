cmake_minimum_required(VERSION 4.4.1)
project(engo-net VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(engo-net
    main/main.cpp
    main/engo_net.cpp
    main/engo_server.cpp
    main/engo_client.cpp
    server/tcp_server.cpp
    server/internal_node.cpp
    server/tui/server_tui.cpp
    client/tcp_client.cpp
    client/user.cpp
    client/identity.cpp
    client/tui/client_tui.cpp
    common/logger.cpp
    common/command_parser.cpp
    common/network/connection.cpp
    common/network/socket.cpp
    common/network/packet.cpp
    common/file_handling/filesystem.cpp
    common/file_handling/formed_file.cpp
    common/tui/buffer.cpp
    common/tui/tui.cpp
    common/tui/rect.cpp
    common/tui/message_list.cpp
    common/tui/text_input.cpp
    common/tui/console.cpp
    common/tui/input_handler.cpp
    common/crypto/key.cpp
    common/crypto/ed25519.cpp
    common/crypto/x25519.cpp
)

target_link_libraries(engo-net
    SQLite::SQLite3
    OpenSSL::Crypto
)
