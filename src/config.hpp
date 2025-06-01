#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/fusion/include/adapt_struct.hpp> 
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <type_traits>
#include <iostream>
#include <vector>
#include <cstdint>

namespace my_server {

/**
 *  top‐level my_server settings:
 */
struct ServerConfig {
    std::string filepath_root; // where static files are stored
    std::string proxy_config_path; // where to find the app.conf
};

struct ProxyTarget {
    std::string route; // path to the application, e.g. /chatroom
    std::string host;
    std::uint16_t port;
};


// Recursive helper
template <typename T, int N>
struct ini_loader {
  static bool load(const boost::property_tree::ptree& section, T& obj) {
    using name = boost::fusion::extension::struct_member_name<T, N>;
    using field_type = typename boost::fusion::result_of::value_at_c<T, N>::type;

    const std::string key = name::call();

    try {
      boost::fusion::at_c<N>(obj) = section.get<field_type>(key);
    } catch (...) {
      return false;
    }

    // Recurse
    return ini_loader<T, N + 1>::load(section, obj);
  }
};

// Base case: stop at size<T>
template <typename T>
struct ini_loader<T, boost::fusion::result_of::size<T>::value> {
  static bool load(const boost::property_tree::ptree&, T&) {
    return true;
  }
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
template <typename T>
std::vector<T> load_config(const std::string& conf_file) {
  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(conf_file, pt);

  std::vector<T> result;

  for (const auto& section : pt) {
    T obj;
    if (ini_loader<T, 0>::load(section.second, obj)) {
      result.push_back(std::move(obj));
    } else {
      std::cerr << "Skipping invalid section [" << section.first << "]\n";
    }
  }

  return result;
}

}

BOOST_FUSION_ADAPT_STRUCT(my_server::ServerConfig, filepath_root, proxy_config_path)
BOOST_FUSION_ADAPT_STRUCT(my_server::ProxyTarget, route, host, port)