#include "cache.hpp"

#include <chrono>

namespace my_server {

void Cache::put(const std::string &path, const std::string &content_type, const void *data, std::size_t data_length)
{
    std::lock_guard lock(_mtx);

    auto *e = new Entry{ path, content_type, {}, std::chrono::steady_clock::now()};
    e->content.resize(data_length);
    std::memcpy(e->content.data(), data, data_length);

    _nodes.insert(e);

    // add to hashtable
    _index.put(path, e);

    while (_nodes.count() > _max_size) {
        // get array view of pointers
        void **view = _nodes.array_get();
        auto    n    = _nodes.count();

        // last element is the least-recently-used
        Entry *old_e = static_cast<Entry*>(view[n - 1]);

        // remove from linked list
        _nodes.l_delete(old_e,
            [](void *a, void *b){ return a == b ? 0 : 1; });

        // remove from hash table (returns the same pointer)
        auto opt = _index.erase(old_e->path);
        if (opt.has_value()) {
            Entry *removed = opt.value();
            // free the cache entry
            delete removed;
        }

        // free temporary array
        Llist::array_free(view);
    }

}

Cache::Entry* Cache::get(const std::string &path) {
    std::lock_guard lock(_mtx);

    // lookup in the hash table
    auto opt = _index.get(path);
    if (!opt.has_value()) {
        return nullptr;
    }
    Entry* e = opt.value();

    auto now = std::chrono::steady_clock::now();

    if (now - e->created_at > _ttl) {
        // remove from list and map
        _nodes.l_delete(
            e,
            [](void *a, void *b){ return a == b ? 0 : 1; }
        );
        auto opt = _index.erase(path);
        if (opt.has_value()) {
            Entry *removed = opt.value();
            // free the cache entry
            delete removed;
        }
        return nullptr;  // Http::get_file() will treat this as cache miss
    }

    // move this entry to the head of the LRU list
    // remove and re-insert at front.
    _nodes.l_delete(
        e,
        [](void* a, void* b) { return a == b ? 0 : 1; }
    );
    _nodes.insert(e);

    // return the entry pointer
    return e;
}

void Cache::erase(const std::string &path) {
    std::lock_guard lock(_mtx);
    
    // look up the entry pointer
    auto opt = _index.erase(path);
    if (!opt.has_value()) {
        return; 
    }

    Entry *e = opt.value();

    // remove it from the usage list
    _nodes.l_delete(
        e,
        [](void *a, void *b){ return a == b ? 0 : 1; }
    );

    delete e;
}

    
}