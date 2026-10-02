cmake_minimum_required(VERSION 3.16)

project(linux_tcp_messenger LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_library(protocol STATIC
    src/common/protocol.cpp
)

target_include_directories(protocol PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(server
    src/server/main.cpp
    src/server/server.cpp
    src/server/client_session.cpp
    src/server/user_manager.cpp
    src/server/message_handler.cpp
)
target_link_libraries(server PRIVATE protocol Threads::Threads)

add_executable(client
    src/client/client.cpp
)
target_link_libraries(client PRIVATE protocol)
