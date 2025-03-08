#ifndef HTTP_H
#define HTTP_H

#include <sys/socket.h>

namespace simple_web_server {

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

private:

int _fd;

};


}


#endif