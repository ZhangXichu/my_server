#pragma once

#include <functional>
#include <vector>
#include <llist.hpp>
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
    _buckets(_size) {
        for (int i = 0; i < _size; i++) {
            Llist llist;
            _buckets.push_back(llist);
        }
    }

/**
 * Put to hash table with a string key
*/
void put(KeyType key, ValueType value) 
{
    std::size_t index = HashFn{} (key);
    Llist& llist = _buckets[index % _size];
    Entry entry{
        .key = key,
        .data = value
    };
    if (!llist.append(&entry)){
        std::cerr << "Failed to push to hash table." << std::endl;
        return;
    }
    _num_entries++;
    return;
}


private:

int _size;
int _num_entries;
std::vector<Llist> _buckets;

};

}