
#include "server_ws_http.hpp"

namespace my_server {

static std::atomic<bool> keep_running{true};
static void signal_handler(int) { keep_running = false; }
    
/**
 * Read http request and decide if this is a WS upgrade.
 * If so, run websocker server.
 * Otherwise, serialize headers+body back into raw bytes and call
 * Http::handle_http_request().
 */
static void handle_one(boost::asio::ip::tcp::socket socket, Cache &cache, ThreadPool &pool, Http &http)
{
    namespace beast = boost::beast;
    namespace ws    = beast::websocket;

    beast::flat_buffer buffer;

    // read the *entire* HTTP request (headers + body)
    beast::http::request<beast::http::string_body> req;
    beast::http::read(socket, buffer, req);

    // detect WebSocket upgrade
    if(ws::is_upgrade(req)) {
        ws::stream<boost::asio::ip::tcp::socket> wsock{std::move(socket)};
        wsock.accept(req);

        // TODO: replace with chatroom
        for(;;) {
            boost::beast::flat_buffer msg;
            wsock.read(msg);
            wsock.text(wsock.got_text());
            wsock.write(msg.data());
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
                                    int ttl_seconds)
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    Cache cache(cache_size, hash_buckets, std::chrono::seconds{ttl_seconds});
    ThreadPool pool(num_threads);
    Http http;

    boost::asio::io_context ioc{1};
    unsigned short port_us = static_cast<unsigned short>(port);
    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::tcp::v4(),
        port_us
    );

    boost::asio::ip::tcp::acceptor acceptor(ioc, endpoint);
    std::cout << "Listening on port " << port << " (HTTP + WS upgrade)\n";

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
                    std::ref(http))
            .detach();
    }

    std::cout << "Shutdown signal received, exiting.\n";
}

}