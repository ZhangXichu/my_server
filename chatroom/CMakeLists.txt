cmake_minimum_required(VERSION 3.15)
project(chatroom)

add_library(chatroom SHARED main.cpp)

find_package(uwebsockets REQUIRED)

set_target_properties(chatroom PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

target_include_directories(chatroom PRIVATE
  ${CMAKE_SOURCE_DIR}/src 
)

target_link_libraries(chatroom PRIVATE uwebsockets::uwebsockets)
target_include_directories(chatroom PRIVATE ${uwebsockets_INCLUDE_DIRS})