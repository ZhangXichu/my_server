#include <gtest/gtest.h>
#include "hashtable.hpp"

TEST(HashTable, put)
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