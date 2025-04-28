#include <gtest/gtest.h>
#include "hashtable.hpp"

TEST(HashTable, put_and_get)
{
    my_server::HashTable<std::string, int> ht(128);

    std::string key = "key_1";
    int value = 11;

    std::string key_2 = "key_2";
    int value_2 = 12;

    ht.put(key, value);
    ht.put(key_2, value_2);

    auto got1 = ht.get(key);
    ASSERT_TRUE(got1.has_value());
    EXPECT_EQ(got1.value(), value);

    auto got2 = ht.get(key_2);
    ASSERT_TRUE(got2);
    EXPECT_EQ(*got2, value_2);

    auto missing = ht.get("does_not_exist");
    EXPECT_FALSE(missing.has_value());
}

TEST(HashTable, erase)
{
    my_server::HashTable<std::string, int> ht(128);

    ht.put("a", 1);
    ht.put("b", 2);

    auto erased = ht.erase("a");
    ASSERT_TRUE(erased.has_value());
    EXPECT_EQ(erased.value(), 1);

    auto checkA = ht.get("a");
    EXPECT_FALSE(checkA.has_value());

    auto checkB = ht.get("b");
    ASSERT_TRUE(checkB.has_value());
    EXPECT_EQ(checkB.value(), 2);

    auto noKey = ht.erase("does_not_exist");
    EXPECT_FALSE(noKey.has_value());
}

TEST(HashTable, foreach)
{
    my_server::HashTable<std::string, int> ht(128);

    ht.put("x", 5);
    ht.put("y", 10);
    ht.put("z", 15);

    std::map<std::string,int> collected;
    ht.foreach([&](const std::string& key, int& value) {
        collected[key] = value;
    });

    EXPECT_EQ(collected.size(), 3u);
    EXPECT_EQ(collected["x"], 5);
    EXPECT_EQ(collected["y"], 10);
    EXPECT_EQ(collected["z"], 15);

    ht.foreach([](const std::string& /*key*/, int& value) {
        value *= 2;
    });

    EXPECT_EQ(*ht.get("x"), 10);
    EXPECT_EQ(*ht.get("y"), 20);
    EXPECT_EQ(*ht.get("z"), 30);
}
