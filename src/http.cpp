#include <iostream>
#include <cstring>
#include <string>    
#include <sstream>
#include <random>
#include <fcntl.h>                                                                                                                                           

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
// Part 2 Task 3
void Http::get_file(int fd, Cache &cache, const std::string& request_path)
{
    std::string path = request_path;
    if (!path.empty() && path.front() == '/')
        path.erase(0, 1);

    // check cache
    if (auto *entry = cache.get(path)) {
        // cache hit
        std::cout << "[CACHE HIT]  " << path << "\n";

        send_response(
            fd,
            "HTTP/1.1 200 OK",
            entry->content_type,
            entry->content.data(),
            static_cast<int>(entry->content.size())
        );
        return;
    }

    // cache miss
    std::cout << "[CACHE MISS] " << path << "\n";

    try {
        File file(_filepath_root + request_path);

        std::string content_type = mime_type_get(request_path);

        // add to cache
        cache.put(
            path,
            content_type,
            file.data().data(),
            file.size()
        );

        send_response(
            fd,
            "HTTP/1.1 200 OK",
            content_type,
            file.data().data(),
            static_cast<int>(file.size())
        );
    } 
    catch (const std::runtime_error &e) {
        std::cerr << "Error loading file: " << e.what() << std::endl;
        resp_404(fd);
    }
}


void Http::handle_http_request(int fd, Cache &cache)
{
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
    } else if (method == "POST") { // Stretch goals 1
        post_save(fd, cache, url, request, bytes_recvd);
    }
    else {
        std::cerr << "Method not supported" << std::endl;
        resp_404(fd);
    }

    return;
}

char *Http::find_start_of_body(char *header)
{
    char *p = header;
    while (*p) {
        // detect the first newline
        int nl1len = 0;
        if (p[0] == '\r' && p[1] == '\n') {
            nl1len = 2;
        }
        else if (p[0] == '\n' || p[0] == '\r') {
            nl1len = 1;
        }
        else {
            ++p;
            continue;
        }
        // detect the second newline immediately after the first
        char *q = p + nl1len;
        int nl2len = 0;
        if (q[0] == '\r' && q[1] == '\n') {
            nl2len = 2;
        }
        else if (q[0] == '\n' || q[0] == '\r') {
            nl2len = 1;
        }
        else {
            // only one newline in a row — keep scanning
            ++p;
            continue;
        }
        // we found two newlines in a row -> end of headers.
        return q + nl2len;
    }
    // no header/body boundary found
    return nullptr;
}

void Http::post_save(int fd, Cache &cache, const std::string &url, char *request_buf, int bytes_recvd)
{
    // locate start of body
    char *body = find_start_of_body(request_buf);
    if (!body) {
        resp_404(fd);
        return;
    }
    int header_len = static_cast<int>(body - request_buf);
    int body_len   = bytes_recvd - header_len;

    // determine filesystem path
    std::string path = url;
    if (!path.empty() && path.front() == '/')
        path.erase(0,1);
    std::string full_path = _filepath_root + path;

    // write body to disk
    int out = open(full_path.c_str(),
                   O_CREAT | O_WRONLY | O_TRUNC,
                   0666);
    if (out < 0) {
        perror("open");
        resp_404(fd);
        return;
    }
    ssize_t written = write(out, body, body_len);
    close(out);
    if (written < 0 || written != body_len) {
        perror("write");
        resp_404(fd);
        return;
    }

    (void) cache;
    // cache.delete(path);

    const std::string ok = "OK";
    send_response(fd,
                  "HTTP/1.1 200 OK",
                  "text/plain",
                  ok.c_str(),
                  static_cast<int>(ok.size()));
}

}