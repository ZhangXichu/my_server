
#include "server_ws_http.hpp"

namespace my_server {

static std::atomic<bool> keep_running{true};
static void signal_handler(int) { keep_running = false; }

static void proxy_websocket(boost::asio::ip::tcp::socket client, 
    boost::asio::ip::tcp::socket upstream,
    std::string host,
    std::string target,
    boost::beast::http::request<boost::beast::http::string_body> const& req) 
{
    namespace beast = boost::beast;
    namespace ws    = beast::websocket;
    using tcp       = boost::asio::ip::tcp;

    // Wrap them into Beast WebSocket streams
    ws::stream<tcp::socket> client_ws{std::move(client)};
    ws::stream<tcp::socket> server_ws{std::move(upstream)};

    // 1) Finish the client handshake
    client_ws.accept(req);

    // 2) Do the upstream handshake
    server_ws.handshake(host, target);

    // 3) Thread #1: read from client -> write to server
    std::thread t([&]()
    {
        boost::system::error_code ec;
        while (!ec) {
            beast::flat_buffer msg;
            client_ws.read(msg, ec);
            if (ec) {
                std::cerr << "[proxy] client -> server read error: " << ec.message() << "\n";
                break;
            }
            server_ws.text(client_ws.got_text());
            server_ws.write(msg.data(), ec);

            std::string payload = beast::buffers_to_string(msg.data());
            std::cout << "[client -> server] " << payload << std::endl;

            if (ec) {
                std::cerr << "[proxy] server write error: " << ec.message() << "\n";
                break;
            }
        }
    });

    // 4) Main thread: read from server → write to client
    {
        boost::system::error_code ec;
        while (!ec) 
        {
            beast::flat_buffer msg;
            server_ws.read(msg, ec);
            if (ec) {
                std::cerr << "[proxy] server -> client read error: " << ec.message() << "\n";
                break;
            }
            client_ws.text(server_ws.got_text());
            client_ws.write(msg.data(), ec);

            std::string payload = beast::buffers_to_string(msg.data());
            std::cout << "[server -> client] " << payload << std::endl;
            
            if (ec) {
                std::cerr << "[proxy] client write error: " << ec.message() << "\n";
                break;
            }
        }
    }

    // clean up
    t.join();
    beast::error_code ec;
    client_ws.close(ws::close_code::normal, ec);
    server_ws.close(ws::close_code::normal, ec);
}
    
/**
 * Read http request and decide if this is a WS upgrade.
 * If so, run websocker server.
 * Otherwise, serialize headers+body back into raw bytes and call
 * Http::handle_http_request().
 */
static void handle_one(boost::asio::ip::tcp::socket socket, Cache &cache, 
    ThreadPool &pool, 
    Http &http,
    const std::vector<ProxyTarget>& proxy_targets)
{
    namespace beast = boost::beast;
    namespace ws    = beast::websocket;
    using tcp       = boost::asio::ip::tcp;

    beast::flat_buffer buffer;

    // read the entire HTTP request (headers + body)
    beast::http::request<beast::http::string_body> req;
    beast::http::read(socket, buffer, req);

    std::string host{ req[beast::http::field::host] };
    std::string path{ req.target() };
    
    std::cout << "[DEBUG] incoming path: " << path << std::endl;

    // detect WebSocket upgrade
    for (auto const &pt : proxy_targets) {
        if(ws::is_upgrade(req) && path.rfind(pt.route, 0) == 0) 
        {
            tcp::socket upstream{socket.get_executor()};
            upstream.connect({
                boost::asio::ip::make_address(pt.host), 
                pt.port
            });

            // b) Hand off both sockets + host/target into the proxy helper
            proxy_websocket(
                std::move(socket),
                std::move(upstream),
                std::move(host),
                std::move(path),
                req
            );
            return;
        }
    }

    // otherwise re-serialize header+any body bytes
    std::ostringstream oss;
    oss << req;                  // writes request-line + headers + "\r\n"
    oss << "\r\n";               // ensure blank line
    std::string raw = oss.str();

    std::cout 
        << "————— Raw Re-serialized Request —————\n"
        << raw 
        << "\n——————————————————————————————\n";

    auto sock_ptr = std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));

    pool.enqueue([sock_ptr, raw = std::move(raw), &cache, &http]() mutable {
        int fd = sock_ptr->native_handle();
        http.handle_http_request(fd, cache, &raw);
        ::close(fd);
    });
}
    
/**
 * main server loop: set up signal handler, dispatch threads,
 * then do a blocking accept() on a Beast TCP acceptor, handing each
 * socket to handle_one().
 */
void run_ws_http_server(int port,
                        std::size_t num_threads,
                        std::size_t cache_size,
                        int hash_buckets,
                        int ttl_second,
                        std::string server_files_root,
                        std::string app_config_filepath)
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    Cache cache(cache_size, hash_buckets, std::chrono::seconds{ttl_second});
    ThreadPool pool(num_threads);
    Http http(server_files_root);

    boost::asio::io_context ioc{1};
    unsigned short port_us = static_cast<unsigned short>(port);
    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::tcp::v4(),
        port_us
    );

    boost::asio::ip::tcp::acceptor acceptor(ioc, endpoint);
    std::cout << "Listening on port " << port << " (HTTP + WS upgrade)\n";

    auto proxy_targets = load_config<ProxyTarget>(app_config_filepath);

    while (keep_running) {
        boost::system::error_code ec;
        boost::asio::ip::tcp::socket socket{ioc};
        acceptor.accept(socket, ec);
        if(ec) {
            if(!keep_running && ec == boost::asio::error::interrupted) break;
            std::cerr << "accept: " << ec.message() << "\n";
            continue;
        }
  
        std::thread(&handle_one,
                    std::move(socket),
                    std::ref(cache),
                    std::ref(pool),
                    std::ref(http),
                    proxy_targets)
            .detach();
    }

    std::cout << "Shutdown signal received, exiting.\n";
}

}