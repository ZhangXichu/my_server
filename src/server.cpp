#include <thread>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "net.hpp"
#include "http.hpp"
#include "thread_pool.hpp"

#define PORT "3496"

std::atomic<bool> keep_running{true};

void signal_handler(int) {
    keep_running = false;
}

int main()
{
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 
    
    int new_fd; // file describer of new connection
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];
    
    my_server::Net net(PORT);

    int listen_fd = net.get_socket_fd();

    if (listen_fd < 0)
    {
        std::cerr << "webserver: fatal error getting listening socket" << std::endl;
        return 1;
    }

    std::cout << "webserver: waiting for connections on port " << PORT << std::endl;

    my_server::Cache cache(10, 128);
    auto http = my_server::Http();

    unsigned int hc = std::thread::hardware_concurrency();
    std::cout << "Hardware reports " << hc << " concurrent threads.\n";
    my_server::ThreadPool pool(hc);

    while(keep_running)
    {
        socklen_t sin_size{sizeof(their_addr)};

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        new_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd < 0)
        {
            if (!keep_running) {
                break;
            }
            std::cerr << "accept" << std::endl;
            continue;
        }

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family, 
            net.get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

        // new_fd is a new socket descriptor for the new connection.
        // listen_fd is still listening for new connections.
        // http.handle_http_request(new_fd, cache);
        // close(new_fd);
        pool.enqueue([new_fd, &cache, &http](){
            http.handle_http_request(new_fd, cache);
            close(new_fd);
        });

    }
    
    return 0;
}