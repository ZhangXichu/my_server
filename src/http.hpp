#ifndef HTTP_H
#define HTTP_H

#include <sys/socket.h>

#include "file.hpp"
#include "cache.hpp"

namespace my_server {

class Http {

public:

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
void handle_http_request(int fd, Cache &cache, const std::string* raw_data);

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, Cache &cache, const std::string& request_path);

/**
 * Send a 404 response
 */
void resp_404(int fd);

/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd);

/**
 * Locate the first byte of the HTTP message body, i.e. the
 * character immediately following the end of the header.
 * Handles “newlines” of \r\n, \n, or \r in any combination.
 *
 * @param header  a null-terminated buffer containing the full request
 * @return pointer into header[] at the first byte of the body, or nullptr
 *         if no complete header-body boundary is found
 */
char *find_start_of_body(char *header);

/**
 * Handle a POST request to save the request body as a file.
 * URL is the “/foo.txt” path; request_buf holds the full HTTP request,
 * and bytes_recvd is the total bytes read into request_buf.
 */
void post_save(int fd, Cache &cache, const std::string &url, char *request_buf, int bytes_recvd);

private:

int _fd;
std::string _filepath_root = "/home/xichuz/workspace/my_server/server_files/"; // TODO: make this configurable

};


}


#endif