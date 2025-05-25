#include <gtest/gtest.h>
#include <fstream>
#include "config.hpp"

TEST(ConfigTest, ParsesMultipleSections) 
{
    const std::string fname = "test_upstreams.conf";
    std::ofstream ofs(fname);
    ofs << R"(
        ; comment before
        [chatroom]
        route = /chat
        host   = 127.0.0.1
        port   = 8080

        [stream]
        route = /stream
        host   = example.com
        port   = 9000
        )";
    ofs.close();

    auto v = my_server::load_proxy_targets(fname);

    ASSERT_EQ(v.size(), 2u);

    EXPECT_EQ(v[0].route, "/chat");
    EXPECT_EQ(v[0].host,   "127.0.0.1");
    EXPECT_EQ(v[0].port,   8080u);

    EXPECT_EQ(v[1].route, "/stream");
    EXPECT_EQ(v[1].host,   "example.com");
    EXPECT_EQ(v[1].port,   9000u);
}