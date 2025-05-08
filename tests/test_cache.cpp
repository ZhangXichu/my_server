#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <chrono>

#include "cache.hpp"

static std::string to_string(const std::vector<std::byte> &v) {
    return std::string(reinterpret_cast<const char*>(v.data()), v.size());
}

TEST(Cache, put_and_get) {
    my_server::Cache cache(3, 128);

    const std::string path1 = "one.txt";
    const std::string ct1   = "text/plain";
    const char    data1[]   = "HELLO";
    cache.put(path1, ct1, data1, sizeof(data1) - 1);

    auto *e1 = cache.get(path1);
    ASSERT_NE(e1, nullptr);
    EXPECT_EQ(e1->path, path1);
    EXPECT_EQ(e1->content_type, ct1);
    EXPECT_EQ(to_string(e1->content), std::string(data1));

    // second item
    const std::string path2 = "two.bin";
    const std::string ct2   = "application/octet-stream";
    const char    data2[]   = "\x01\x02\x03";
    cache.put(path2, ct2, data2, sizeof(data2));

    auto *e2 = cache.get(path2);
    ASSERT_NE(e2, nullptr);
    EXPECT_EQ(e2->path, path2);
    EXPECT_EQ(e2->content_type, ct2);
    EXPECT_EQ(e2->content.size(), sizeof(data2));
}

TEST(CacheTest, LruEviction) {
    my_server::Cache cache(2, 128);

    cache.put("A", "t/A", "AAA", 3);
    cache.put("B", "t/B", "BBB", 3);

    // access A, order:   A (MRU), B (LRU)
    ASSERT_NE(cache.get("A"), nullptr);

    // insert C, should evict B
    cache.put("C", "t/C", "CCC", 3);

    EXPECT_NE(cache.get("A"), nullptr); // still there, and accesses A
    EXPECT_EQ(cache.get("B"), nullptr); // B is already evicted
    EXPECT_NE(cache.get("C"), nullptr); // just inserted, and accesses C

    // order now is: C (MRU), A (LRU)
    // insert D, evict A
    cache.put("D", "t/D", "DDD", 3);

    EXPECT_EQ(cache.get("A"), nullptr); // A is already evicted
    EXPECT_NE(cache.get("C"), nullptr); // accesses C
    EXPECT_NE(cache.get("D"), nullptr); // accesses D
}

TEST(CacheTest, TtlNotExpired) {
    using namespace std::chrono_literals;
    // ttl = 200 ms
    my_server::Cache cache(5, 128, std::chrono::milliseconds{200});

    const std::string path = "foo.txt";
    const std::string ct   = "text/plain";
    const char    data[]   = "ABC";
    cache.put(path, ct, data, sizeof(data) - 1);

    // wait less than ttl
    std::this_thread::sleep_for(100ms);

    auto *e = cache.get(path);
    ASSERT_NE(e, nullptr) << "Entry should still be valid before TTL expires";
    EXPECT_EQ(to_string(e->content), "ABC");
}

TEST(CacheTest, TtlExpired) {
    using namespace std::chrono_literals;
    // ttl = 50 ms
    my_server::Cache cache(5, 128, std::chrono::milliseconds{50});

    const std::string path = "bar.txt";
    const std::string ct   = "text/plain";
    const char    data[]   = "XYZ";
    cache.put(path, ct, data, sizeof(data) - 1);

    // wait longer than TTL
    std::this_thread::sleep_for(100ms);

    auto *e = cache.get(path);
    EXPECT_EQ(e, nullptr) << "Entry should be expired after TTL";
}