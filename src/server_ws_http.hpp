#ifndef SERVER_WS_HTTP_H
#define SERVER_WS_HTTP_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <csignal>
#include <iostream>
#include <atomic>
#include <string>
#include <thread>
#include "cache.hpp"
#include "thread_pool.hpp"
#include "http.hpp"

namespace my_server {

/**
 * Serve both plain HTTP and WebSocket upgrade requests on the same port.
 * This function blocks until the server is shut down (e.g. via SIGINT).
 * 
 * @param port         TCP port to listen on (e.g. 3496)
 * @param num_threads  how many worker threads to spawn
 * @param cache_size   max entries in the LRU cache
 * @param hash_buckets buckets for the cache’s hashtable
 * @param ttl_seconds  time‐to‐live for cache entries in seconds
 */
void run_ws_http_server(int port,
    std::size_t num_threads,
    std::size_t cache_size,
    int hash_buckets,
    int ttl_seconds);

}

#endif