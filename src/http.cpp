#include <iostream>
#include <cstring>
#include <string>    
#include <sstream>
#include <random>                                                                                                                                         

#include "http.hpp"
#include "mime.hpp"

namespace my_server {

// Part 1 Task 1
int Http::send_response(int fd, const std::string &header, const std::string &content_type, const void *body, int content_length)
{
    // const int max_response_size = 262144;
    // char response[max_response_size];

    // Build HTTP response and store it in response
    std::string response_header = header + "\r\n" +
                                  "Content-Length: " + std::to_string(content_length) + "\r\n" +
                                  "Content-Type: " + content_type + "\r\n" +
                                  "\r\n";

    // Send the header
    int rv = send(fd, response_header.c_str(), response_header.size(), 0);
    
    if (rv < 0) {
        perror("send response header");
        return rv;
    }

    if (content_length > 0)
    {
        rv = send(fd, body, content_length, 0);
        if (rv < 0) {
            perror("send response body");
            return rv;
        }
    }

    return rv;
}

// Part 1 Task 3
void Http::get_d20(int fd)
{
    std::random_device rd; 
    std::mt19937          gen(rd());
    std::uniform_int_distribution<int> dist(1, 20); 
    int roll = dist(gen);

    std::string body = std::to_string(roll);

    int rv = send_response(
        fd,
        "HTTP/1.1 200 OK",
        "text/plain",
        body.c_str(),
        static_cast<int>(body.size())
    );

    if (rv < 0) {
        perror("get_d20 send_response");
    }
}

void Http::resp_404(int fd)
{
    std::string not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(fd, not_found.c_str(), not_found.size(), 0);
}

// Part 1 Task 2
void Http::get_file(int fd, struct cache *cache, const std::string& request_path)
{
    (void) cache;

    try {
        File file(_filepath_root + request_path);

        std::string content_type = my_server::mime_type_get(request_path);

        send_response(
            fd,
            "HTTP/1.1 200 OK",
            content_type,
            file.data().data(),
            static_cast<int>(file.size())
        );
    } catch (const std::runtime_error &e) {
        std::cerr << "Error loading file: " << e.what() << std::endl;
        resp_404(fd);
    }
}


void Http::handle_http_request(int fd, struct cache *cache)
{
    (void) cache;

    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        std::cerr << "recv" << std::endl;
        return;
    }

    std::istringstream iss(request);

    std::string method, url, version;

    if (iss >> method >> url >> version) {
        std::cout << "Parsed values: " << method << ", " << url << ", " << version << std::endl;
    } else {
        std::cerr << "Failed to parse input" << std::endl;
    }

    if (method == "GET")
    {
        if (url == "/d20") {
            get_d20(fd);
        } else {
            get_file(fd, cache, url);
        }
    }

    // TODO
    // If GET, handle the get endpoints

    //    Check if it's /d20 and handle that special case
    //    Otherwise serve the requested file by calling get_file()


    // (Stretch) If POST, handle the post request

    return;

}

}