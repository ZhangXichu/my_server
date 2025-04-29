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
    _buckets(_size) {}

~HashTable() {
    for (auto &bucket : _buckets) {
        bucket.foreach(
            [](void* rawData, void*) {
                delete static_cast<Entry*>(rawData);
            },
            nullptr
        );
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

/**
 * Erase by key.
 *
 * Returns the old value if present, or std::nullopt if not.
 * Does not free the ValueType itself.
 */
[[nodiscard]]
std::optional<ValueType> erase(const KeyType& key) {
    std::size_t h   = HashFn{}(key);
    std::size_t idx = h % _buckets.size();

    void* raw = _buckets[idx].l_delete(
        const_cast<KeyType*>(&key),    
        [](void* a, void* b) -> int {   
            auto* kp  = static_cast<KeyType*>(a);
            auto* ent = static_cast<Entry*>(b);
            return (*kp == ent->key) ? 0 : 1;
        }
    );

    if (!raw) {
        return std::nullopt; 
    }

    Entry* ent    = static_cast<Entry*>(raw);
    ValueType val = std::move(ent->data);
    delete ent;
    --_num_entries;
    return val;
}


/**
 * Apply `f(key,data)` to every entry in the table, in no particular order.
 */
template<typename F>
void foreach(F f) {
    for (std::size_t i = 0; i < _buckets.size(); ++i) {
        Llist& bucket = _buckets[i];
        bucket.foreach(
            [](void* raw_data, void* raw_f) {
                auto* ent  = static_cast<Entry*>(raw_data);
                auto& f    = *static_cast<F*>(raw_f);
                f(ent->key, ent->data);
            },
            &f
        );
    }
}


private:

int _size;
int _num_entries;
std::vector<Llist> _buckets;

};

}