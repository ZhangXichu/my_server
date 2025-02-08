#include "net.hpp"

namespace simple_web_server {

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

int Net::get_listener_socket(const std::string port)
{
    int sockfd;
    struct addrinfo hints{};
    struct addrinfo *servinfo, *p;
    // int yes = 1;
    int rv;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // This block of code looks at the local network interfaces and
    // tries to find some that match our requirements (namely either
    // IPv4 or IPv6 (AF_UNSPEC) and TCP (SOCK_STREAM) and use any IP on
    // this machine (AI_PASSIVE).

    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
    }

    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        std::cout << "Socket FD: " << sockfd
                  << ", ai_family: " << p->ai_family
                  << ", ai_socktype: " << p->ai_socktype
                  << ", ai_protocol: " << p->ai_protocol << std::endl;
    }

    freeaddrinfo(servinfo);

    return 0;
}   

}

