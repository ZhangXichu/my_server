cmake_minimum_required(VERSION 3.20)

project(chatroom VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(server 
    src/server.cpp
    src/net.c)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
