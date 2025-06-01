#include "server_ws_http.hpp"

int main()
{
    constexpr int  PORT          = 3496;
    unsigned int   hc            = std::thread::hardware_concurrency();
    std::size_t    cache_size    = 10;
    int            hash_buckets  = 128;
    int            ttl_seconds   = 300; 

    std::cout << "Hardware reports " << hc << " hardware threads\n";

    // TODO: make this input argument
    auto v = my_server::load_config<my_server::ServerConfig>("/home/xichuz/workspace/my_server/configs/my_server.conf");

    my_server::run_ws_http_server(
        PORT, hc, cache_size, hash_buckets, ttl_seconds, v[0].filepath_root, v[0].proxy_config_path
    );
    return 0;
}