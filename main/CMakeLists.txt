add_executable(engo-net
    engo_net.cpp
    engo_client.cpp
    engo_server.cpp
    main.cpp
)

target_link_libraries(engo-net PRIVATE server-lib)
target_link_libraries(engo-net PRIVATE client-lib)
target_link_libraries(engo-net PRIVATE common-lib)

target_link_libraries(engo-net PRIVATE
    SQLite::SQLite3
    OpenSSL::Crypto
)
