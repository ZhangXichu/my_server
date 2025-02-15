#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "net.hpp"

#define PORT "3496"

int main()
{
    // int newfd; // file describer of new connection
    // struct sockaddr_storage their_addr;
    
    simple_web_server::Net net(PORT);

    int socket_fd = net.get_socket_fd();

    std::cout << "socket fd: " << socket_fd << std::endl;
}