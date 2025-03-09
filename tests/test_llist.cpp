#include <gtest/gtest.h>
#include "llist.hpp" 

TEST(Llist, InserAndFind) 
{
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
    EXPECT_EQ(list.count(), 3);
}

TEST(Llist, Delete)
{
    my_server::Llist list;
    int a = 100;

    list.insert(&a);

    auto cmp = [](void* key, void* data) -> int {
        int* key_int = static_cast<int*>(key);
        int* node_int = static_cast<int*>(data);
        return (*key_int - *node_int);
    };

    void* removed = list.l_delete(&a, cmp);
    ASSERT_NE(removed, nullptr);
    EXPECT_EQ(*static_cast<int*>(removed), 100);
    EXPECT_EQ(list.count(), 0);
}

TEST(Llist, ArrayGet)
{
    my_server::Llist list;

    int a = 1, b = 2;

    list.insert(&a);
    list.append(&b);

    void** arr = list.array_get();
    ASSERT_NE(arr, nullptr);

    EXPECT_EQ(arr[0], &a);
    EXPECT_EQ(arr[1], &b);
    EXPECT_EQ(arr[2], nullptr);

    my_server::Llist::array_free(arr);
}

TEST(Llist, ForEach)
{
    my_server::Llist list;

    int a = 5, b = 10, c = 15;

    list.insert(&a);
    list.append(&b);
    list.append(&c);

    int sum = 0;
    auto add_func = [](void* data, void* arg) {
        int* value = static_cast<int*>(data);
        int* sum_ptr = static_cast<int*>(arg);
        *sum_ptr += *value;
    };

    list.foreach(add_func, &sum);
    EXPECT_EQ(sum, 5 + 10 + 15);
}