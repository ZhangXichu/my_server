
#include "server_ws_http.hpp"

namespace my_server {

static std::atomic<bool> keep_running{true};
static void signal_handler(int) { keep_running = false; }
    
/**
 * Read just the HTTP headers to decide if this is a WS upgrade.
 * If so, run websocker server.
 * Otherwise, serialize headers+body back into raw bytes and call
 * Http::handle_http_request().
 */
static void handle_one(boost::asio::ip::tcp::socket socket, Cache &cache, ThreadPool &pool, Http &http)
{
    boost::beast::flat_buffer buffer;

    // read header only
    boost::beast::http::request_parser<boost::beast::http::string_body> parser;
    boost::beast::http::read_header(socket, buffer, parser);
    auto req = parser.get(); 

    {
        // Dump the beast::http::request to stdout
        std::ostringstream req_stream;
        req_stream << req;
        std::cout 
          << "————— Parsed HTTP Request —————\n"
          << req_stream.str()
          << "——————————————————————————————\n";
    }

    // detect WebSocket upgrade
    if(boost::beast::websocket::is_upgrade(req)) {
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws{std::move(socket)};
        ws.accept(req);

        // TODO: replace with chatroom
        for(;;) {
            boost::beast::flat_buffer msg;
            ws.read(msg);
            ws.text(ws.got_text());
            ws.write(msg.data());
        }

        return;
    }

    // otherwise re-serialize header+any body bytes
    std::string raw = std::string{req.method_string()} + " " +
                        std::string{req.target()} + " HTTP/" +
                        std::to_string(req.version()/10) + "." +
                        std::to_string(req.version()%10) + "\r\n";
    for(auto const& field : req) {
        raw += std::string{ field.name_string() } + ": " +
            std::string{ field.value() } + "\r\n";
    }
    raw += "\r\n";
    if(! req.body().empty()) raw += req.body();

    std::cout 
        << "————— Raw Re-serialized Request —————\n"
        << raw 
        << "\n——————————————————————————————\n";

    // push those bytes back so your recv() in Http sees them
    int orig_fd = socket.native_handle();
    int fd = dup(orig_fd);
    // send(fd, raw.data(), raw.size(), MSG_PEEK);

    // handle as before, on the thread pool
    pool.enqueue([fd, raw, &cache, &http](){
        http.handle_http_request(fd, cache, &raw);
        close(fd);
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