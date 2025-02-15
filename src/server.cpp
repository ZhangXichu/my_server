#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "net.hpp"

#define PORT "3496"

int main()
{
    int new_fd; // file describer of new connection
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];
    
    simple_web_server::Net net(PORT);

    int listen_fd = net.get_socket_fd();

    if (listen_fd < 0)
    {
        std::cerr << "webserver: fatal error getting listening socket" << std::endl;
        return 1;
    }

    std::cout << "webserver: waiting for connections on port " << PORT << std::endl;

    while(true)
    {
        socklen_t sin_size{sizeof(their_addr)};

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        new_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1)
        {
            std::cerr << "accept" << std::endl;
            continue;
        }

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family, 
            net.get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

        // new_fd is a new socket descriptor for the new connection.
        // listen_fd is still listening for new connections.

        close(new_fd);
    }
}