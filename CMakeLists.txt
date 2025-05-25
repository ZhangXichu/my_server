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
    src/thread_pool.cpp
    src/server_ws_http.cpp
    src/config.cpp
)

find_package(Boost 1.87 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

add_executable(server 
    src/server.cpp
    $<TARGET_OBJECTS:libserver>)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

add_executable(
    tests
    tests/test_llist.cpp
    tests/test_hashtable.cpp
    tests/test_cache.cpp
    tests/test_http.cpp
    tests/test_config_parse.cpp
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
  OpenSSL::Crypto      
)

target_link_libraries(libserver
    PUBLIC
    OpenSSL::Crypto
    OpenSSL::SSL
)

target_link_libraries(server
  PRIVATE Boost::system Boost::thread
  OpenSSL::SSL
  OpenSSL::Crypto
)

include(GoogleTest)
gtest_discover_tests(tests)
