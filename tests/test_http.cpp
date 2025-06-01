#include <gtest/gtest.h>

#include "http.hpp"

using my_server::Http;

static constexpr const char* path = "/home/xichuz/workspace/my_server/server_files";

static char* call_find(Http &http, const std::string &req) {
    // std::string::data() returns a pointer to a mutable buffer in C++17+
    // so it's safe to cast away const here for our API.
    return http.find_start_of_body(const_cast<char*>(req.data()));
}

TEST(HttpFindBodyTest, CRLFCRLF) {
    Http http(path);
    std::string req =
        "POST /path HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
        "ABCD";
    char *body = call_find(http, req);
    ASSERT_NE(body, nullptr);
    EXPECT_STREQ(body, "ABCD");
    auto pos = req.find("\r\n\r\n");
    EXPECT_EQ(body - req.data(), static_cast<int>(pos + 4));
}

TEST(HttpFindBodyTest, LF_LF) {
    Http http(path);
    std::string req =
        "POST /path HTTP/1.1\n"
        "Host: example.com\n"
        "Content-Length: 3\n"
        "\n"
        "XYZ";
    char *body = call_find(http, req);
    ASSERT_NE(body, nullptr);
    EXPECT_STREQ(body, "XYZ");
    auto pos = req.find("\n\n");
    EXPECT_EQ(body - req.data(), static_cast<int>(pos + 2));
}

TEST(HttpFindBodyTest, CR_CR) {
    Http http(path);
    std::string req =
        "POST /foo HTTP/1.0\r"
        "Header: val\r"
        "\r"
        "123";
    char *body = call_find(http, req);
    ASSERT_NE(body, nullptr);
    EXPECT_STREQ(body, "123");
    auto pos = req.find("\r\r");
    EXPECT_EQ(body - req.data(), static_cast<int>(pos + 2));
}

TEST(HttpFindBodyTest, MixedNewlines) {
    Http http(path);
    std::string req =
        "POST /mixed HTTP/1.1\r\n"
        "A: 1\n"
        "\n"
        "HELLO";
    char *body = call_find(http, req);
    ASSERT_NE(body, nullptr);
    EXPECT_STREQ(body, "HELLO");
    // the sequence is "\r\n" then "A: 1\n" then "\n"
    // find_start should spot the "\n\n" at the end of headers:
    auto pos = req.find("\n\n");
    EXPECT_EQ(body - req.data(), static_cast<int>(pos + 2));
}

TEST(HttpFindBodyTest, NoTerminator) {
    Http http(path);
    std::string req =
        "GET /no-body HTTP/1.1\r\n"
        "Host: example.com\r\n";
    char *body = call_find(http, req);
    EXPECT_EQ(body, nullptr);
}