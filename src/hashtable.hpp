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
    std::size_t index = HashFn{} (key) % _size;

    Llist& llist = _buckets[index];

    Entry* entry = new Entry{ key, value };
    
    if (!llist.append(entry)){
        std::cerr << "Failed to push to hash table." << std::endl;
        return;
    }
    _num_entries++;
    return;
}

/**
 * Get from the hash table with a string key
 */
[[nodiscard]] std::optional<ValueType> get(const KeyType& key) const {
    std::size_t index = HashFn{} (key) % _size;

    Llist& bucket = const_cast<Llist&>(_buckets[index]);

    void* raw = bucket.find(
        const_cast<KeyType*>(&key),
        [](void* a, void* b) -> int {
            auto* kp  = static_cast<KeyType*>(a);
            auto* entry = static_cast<Entry*>(b);
            return (*kp == entry->key) ? 0 : 1;
        }
    );

    if (!raw) {
        return std::nullopt; 
    }

    auto* entry = static_cast<Entry*>(raw);
    return entry->data;
}


private:

int _size;
int _num_entries;
std::vector<Llist> _buckets;

};

}