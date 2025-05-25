#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>

#include "config.hpp"

namespace my_server {

std::vector<ProxyTarget> load_proxy_targets(const std::string &ini_file) {
  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(ini_file, pt);

  std::vector<ProxyTarget> v;
  for (auto &sect : pt) {
    ProxyTarget tgt;
    // section name is ignored
    tgt.route = sect.second.get<std::string>("route", "");
    tgt.host   = sect.second.get<std::string>("host",   "");
    tgt.port   = sect.second.get<uint16_t>("port", 0);

    if (tgt.route.empty() || tgt.host.empty() || !tgt.port) {
      std::cerr << "Skipping invalid proxy section [" 
                << sect.first << "]\n";
    } else {
      v.push_back(std::move(tgt));
    }
  }
  return v;
}

}