#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace my_server {

struct ProxyTarget {
    std::string route; // path to the application, e.g. /chatroom
    std::string host;
    std::uint16_t port;
};

/**
 * Read an ini‐style config file of ProxyTarget sections:
 *
 *   [chatroom]
 *   route = /chatroom
 *   host   = 127.0.0.1
 *   port   = 9001
 *
 * Returns a vector of ProxyTarget entries.
 * Throws on io or parse errors.
 */
std::vector<ProxyTarget> load_proxy_targets(const std::string &ini_file);

}