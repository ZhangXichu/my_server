cmake_minimum_required(VERSION 3.15)
project(chatroom)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(chatroom main.cpp)

find_package(uwebsockets REQUIRED)

target_link_libraries(chatroom PRIVATE uwebsockets::uwebsockets)
target_include_directories(chatroom PRIVATE ${uwebsockets_INCLUDE_DIRS})