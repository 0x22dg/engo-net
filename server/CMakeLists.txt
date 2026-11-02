add_library(server-lib
    auth_manager.cpp
    data_manager.cpp
    internal_node.cpp
    server_state.cpp
    tcp_server.cpp
    user_registry.cpp
    tui/server_tui.cpp
)

target_include_directories(server-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
