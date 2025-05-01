cmake_minimum_required(VERSION 3.20)

project(chatroom VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/0bdccf4aa2f5c67af967193caf31d42d5c49bde2.zip
)

FetchContent_MakeAvailable(googletest)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_library(libserver OBJECT
    src/net.cpp
    src/http.cpp
    src/file.cpp
    src/mime.cpp
    src/llist.cpp
    src/cache.cpp
)


add_executable(server 
    src/server.cpp
    $<TARGET_OBJECTS:libserver>)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

add_executable(
    tests
    tests/test_llist.cpp
    tests/test_hashtable.cpp
    $<TARGET_OBJECTS:libserver>
)

target_include_directories(
    tests 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(
    tests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
