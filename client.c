#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *message = argv[3];

    // create a UDP socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Clear memory and configure IPv4 and correct port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Chech that IP address is valid and assign it to server_addr
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid address: %s\n", ip);
        exit(EXIT_FAILURE);
    }
    
    // Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Create epoll event and it's file descriptor
    struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Connect epoll file descriptor to the socket as an outgoing event
    event.events = EPOLLOUT;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    int n_sent = 0;
    int n_total = strlen(message);
    while (n_sent < n_total) {
        // wait for socket to be available
        int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_ready == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n_ready; i++) {
            if (events[i].events & EPOLLOUT) {
                // send as much of the message as possible
                int n_bytes = send(sock_fd, message + n_sent, n_total - n_sent, 0);
                if (n_bytes == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                n_sent += n_bytes;
            }
        }
    }

    close(sock_fd);
    return 0;
}
