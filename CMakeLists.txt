cmake_minimum_required(VERSION 4.4.1)
project(engo-net VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(main)
