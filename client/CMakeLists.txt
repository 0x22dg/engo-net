add_library(client-lib
    auth_service.cpp
    identity.cpp
    tcp_client.cpp
    user.cpp
    tui/client_tui.cpp
)

target_include_directories(client-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
