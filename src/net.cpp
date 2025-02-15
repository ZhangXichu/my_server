#include <memory>

#include "net.hpp"

namespace simple_web_server {

struct AddrInfoDeleter {
    void operator()(addrinfo* ai) const {
        if (ai) freeaddrinfo(ai);
    }
};

Net::Net(const std::string& port)
    : _port(port)
{
    _socket_fd = get_listener_socket(port);
}

Net::~Net()
{
    if (_socket_fd >= 0)
    {
        close(_socket_fd);
    }
}

int Net::get_socket_fd()
{
    return _socket_fd;
}

int Net::get_listener_socket(const std::string port)
{
    int sockfd;
    struct addrinfo hints{};
    struct addrinfo *servinfo_raw, *p;
    int yes = 1;
    int rv;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // This block of code looks at the local network interfaces and
    // tries to find some that match our requirements (namely either
    // IPv4 or IPv6 (AF_UNSPEC) and TCP (SOCK_STREAM) and use any IP on
    // this machine (AI_PASSIVE).

    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo_raw)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }

    std::unique_ptr<addrinfo, AddrInfoDeleter> servinfo(servinfo_raw);

    for (p = servinfo.get(); p != nullptr; p = p->ai_next)
    {
        // Try to make a socket based on this candidate interface
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        // SO_REUSEADDR prevents the "address already in use" errors
        // that commonly come up when testing servers.
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            std::cerr << strerror(errno) << std::endl;
            close(sockfd);
            continue;
        }

        // See if we can bind this socket to this local IP address. This
        // associates the file descriptor (the socket descriptor) that
        // we will read and write on with a specific IP address.
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        // If we got here, we got a bound socket and we're done
        break;
    }

    // If p is NULL, it means we didn't break out of the loop, above,
    // and we don't have a good socket
    if (p == nullptr)
    {
        std::cerr << "webserver: failed to find local address: " << std::endl;
        return -3;
    }

    // Start listening. This is what allows remote computers to connect
    // to this socket/IP.
    if (listen(sockfd, _backlog) == -1) 
    {
        close(sockfd);
        return -4;
    }

    return sockfd;
}   

}

