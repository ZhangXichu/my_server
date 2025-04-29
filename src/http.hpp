#ifndef HTTP_H
#define HTTP_H

#include <sys/socket.h>

#include "file.hpp"

namespace my_server {

class Http {

public:

/**
 * Handle HTTP request and send response
 */
// void handle_http_request(struct cache *cache);

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, const std::string &header, const std::string &content_type, const void *body, int content_length);

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache);

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, const std::string& request_path);

/**
 * Send a 404 response
 */
void resp_404(int fd);

/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd);

private:

int _fd;
std::string _filepath_root = "../src/server_files/";

};


}


#endif