#include <mutex>
#include <fstream>
#include <sstream>

#include <uWebSockets/App.h>

std::mutex ws_mtx;
uWS::App *global_app;
// empty custom user data for each WebSocket connection
struct PerSocketData {};
std::vector<uWS::WebSocket<false, true, struct PerSocketData>*> ws_clients;

int main()
{
    int ws_port = 8080;
    std::string html_content;
    std::string file_path = "/home/xichuz/workspace/chat/index.html";
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + file_path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        html_content = buffer.str();
        
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    uWS::App app;

    global_app = &app;

    app.get("/", [&html_content](auto *res, auto * /*req*/) {
            res->writeHeader("Content-Type", "text/html")->end(html_content);
        })
        .ws<struct PerSocketData>("/",{
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 10,
        .open = [](auto *ws) {
            std::cout << "WebSocket client connected" << std::endl;
            ws->subscribe("broadcast");
            
            std::lock_guard<std::mutex> lock(ws_mtx);
            ws_clients.push_back(ws);
        },
        .message = []([[maybe_unused]] auto *ws,  std::string_view message, [[maybe_unused]] uWS::OpCode opCode) 
        {
            global_app->publish("broadcast", message, opCode, false);
        },
        .close = [](auto *ws, [[maybe_unused]] int code, [[maybe_unused]] std::string_view message) 
        {
            std::cout << "WebSocket client disconnected" << std::endl;
            std::lock_guard<std::mutex> lock(ws_mtx);
            ws_clients.erase(std::remove(ws_clients.begin(), ws_clients.end(), ws), ws_clients.end());
        }
    }).listen("0.0.0.0", ws_port, [ws_port](auto *listenSocket) {
        if (listenSocket) {
            std::cout << "WebSocket/HTTP server listening on port 8080:" << ws_port << std::endl;
        }
    });

    app.run();

    // ws_thread.join();
    delete global_app;

    return 0;
}