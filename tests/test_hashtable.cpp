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
}