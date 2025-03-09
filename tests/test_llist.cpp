#include <gtest/gtest.h>
#include "llist.hpp" 

TEST(Llist, inserAndFind) {
    my_server::Llist list;
    int a = 10, b = 20, c = 30;

    list.insert(&a);
    list.append(&b);
    list.append(&c);

    auto cmp = [](void* key, void* data) -> int {
        int* key_int = static_cast<int*>(key);
        int* node_int = static_cast<int*>(data);
        return (*key_int - *node_int);
    };

    void* found = list.find(&b, cmp);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(*static_cast<int*>(found), 20);
}

