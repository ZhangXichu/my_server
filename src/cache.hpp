#pragma once

#include <cstddef>
#include <string>
#include <optional>

#include "hashtable.hpp"

namespace my_server {

class Cache {

public:

struct Entry {
    std::string path;
    std::string content_type;
    std::vector<std::byte> content;
};

Cache(std::size_t max_size, int hashsize)
      : _max_size(max_size), _index(hashsize)
    {}

~Cache() {
    // clean up any remaining entries in the linked list
    void **arr = _nodes.array_get();
    int   cnt = _nodes.count();
    for (int i = 0; i < cnt; ++i) {
        delete static_cast<Entry*>(arr[i]);
    }
    Llist::array_free(arr);
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
void put(const std::string &path, const std::string &content_type, const void *data, std::size_t data_length);

/**
 * Retrieve an entry from the cache by path.
 * If found, moves it to the most‐recently‐used position and returns a pointer.
 * Returns nullptr if not present.
 */
Entry* get(const std::string &path);

private:

std::size_t _max_size;
Llist _nodes; // holds Entry* in MRU->LRU order
HashTable<std::string, Entry*> _index; // map path -> Entry*


};

}