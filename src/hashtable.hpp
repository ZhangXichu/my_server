#pragma once

#include <functional>
#include <vector>
#include <list>
#include <memory>
#include <cstring>
#include <string>

namespace my_server{

template<typename KeyType, typename ValueType, typename HashFn = std::hash<KeyType>>
class HashTable {
public:
struct Entry {
    KeyType key;
    ValueType data;
};

explicit HashTable(int size)
    :_size(size > 0 ? size : 128),
    _num_entries(0),
    _buckets(_size)

private:

int _size;
int _num_entries;
std::vector<std::list<Entry>> _buckets;

};

}