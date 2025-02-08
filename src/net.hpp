#ifndef NET_H
#define NET_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <cstring>

namespace simple_web_server {

class Net {

public:

Net(const std::string& port); // TODO: add backlog
~Net();
int get_listener_socket(const std::string port);

private:

std::string _port;
int _socket_fd;

};


}




#endif