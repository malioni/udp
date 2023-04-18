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
#include <fstream>

#define MAX_EVENTS 10
#define BUF_SIZE 1024

int read_file(const std::string &file_path, std::string &file_contents)
{
    std::ifstream ifss(file_path); // open file
    
    if (ifss.is_open()) // check that file is open
    {
        ifss.seekg(0, std::ios::end); // set the get position to the end of the file
        file_contents.reserve(ifss.tellg()); // reserve space in the string for the entire file
        ifss.seekg(0, std::ios::beg); // set the get position back to the beginning of the file
        file_contents.assign((std::istreambuf_iterator<char>(ifss)),
                              std::istreambuf_iterator<char>()); // read file into string
        ifss.close(); // close file
    }
    else 
    {
        return -1;
    }
    
    return 0;
        
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    std::string file_name(argv[3]);
    
    std::string file_contents;
    // read the file
    if (read_file(file_name, file_contents) < 0)
    {
        perror("Error reading the file");
        exit(EXIT_FAILURE);
    }
    
    std::cout << file_contents << std::endl;

//     // create a UDP socket
//     int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sock_fd == -1) {
//         perror("socket");
//         exit(EXIT_FAILURE);
//     }

//     // Clear memory and configure IPv4 and correct port
//     struct sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port);
    
//     // Chech that IP address is valid and assign it to server_addr
//     if (inet_pton(AF_INET, ip, &server_addr.sin_addr) != 1) {
//         fprintf(stderr, "Invalid address: %s\n", ip);
//         exit(EXIT_FAILURE);
//     }
    
//     // Connect to the server
//     if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
//         perror("connect");
//         exit(EXIT_FAILURE);
//     }

//     // Create epoll event and it's file descriptor
//     struct epoll_event event, events[MAX_EVENTS];
//     int epoll_fd = epoll_create1(0);
//     if (epoll_fd == -1) {
//         perror("epoll_create1");
//         exit(EXIT_FAILURE);
//     }

//     // Connect epoll file descriptor to the socket as an outgoing event
//     event.events = EPOLLOUT;
//     event.data.fd = sock_fd;
//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
//         perror("epoll_ctl");
//         exit(EXIT_FAILURE);
//     }
    
//     // Start the clock for time measurement
//     clock_t start_time = clock();

//     int n_sent = 0;
//     int n_total = strlen(buffer);
//     while (n_sent < n_total) {
//         // wait for socket to be available
//         int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//         if (n_ready == -1) {
//             perror("epoll_wait");
//             exit(EXIT_FAILURE);
//         }

//         for (int i = 0; i < n_ready; i++) {
//             if (events[i].events & EPOLLOUT) {
//                 // send as much of the message as possible
//                 int n_bytes = send(sock_fd, buffer + n_sent, n_total - n_sent, 0);
//                 printf("Sending data\n");
//                 if (n_bytes == -1) {
//                     perror("send");
//                     exit(EXIT_FAILURE);
//                 }
//                 n_sent += n_bytes;
//             }
//         }
//     }
    
//     // End the clock
//     clock_t end_time = clock();
//     double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
//     printf("File transfer speed: %.2f MB/s\n", (double)file_size / elapsed_time / (1024 * 1024));

//     // Close the socket
//     close(sock_fd);
    return 0;
}
