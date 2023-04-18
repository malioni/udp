#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>

#define MAX_EVENTS 10
#define BUF_SIZE 1024

struct file_to_write
{
    std::string name;
    std::string contents;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    std::map<std::string, file_to_write> m_sock_to_file;
    std::stringstream ss;

    int port = atoi(argv[1]);

    // Create a socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Reset memory and specify IPv4, any IP address and port number
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket's name to an address
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // set socket to non-blocking mode
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
    
    // Create epoll event and field descriptor
    struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Specify that events are incoming to the server
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE];
    while (1) {
        int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_ready < 0) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n_ready; i++) {
            if (events[i].data.fd == sock_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                
                int n_bytes = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
                                       (struct sockaddr *)&client_addr, &client_addr_len);
                if (n_bytes < 0) {
                    perror("recvfrom");
                    exit(EXIT_FAILURE);
                }
                
                ss.clear();
                ss << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);
                std::cout << ss.str() << std::endl;

                printf("Received message from %s:%d: %.*s\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                       n_bytes, buffer);
            }
        }
    }

    close(sock_fd);
    close(epoll_fd);
    return 0;
}
