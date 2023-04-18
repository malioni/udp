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
}
        
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    std::string file_name(argv[3]);
    
    std::string file_contents;
    // read the file
    if (read_file(file_name, file_contents) < 0)
    {
        perror("Error reading the file");
        exit(EXIT_FAILURE);
    }

    // create a UDP socket
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure IPv4 and correct port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // set socket to non-blocking mode
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);

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
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) < 0) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    
    // TODO: Send the file name

    int n_sent = 0;
    int n_total = file_contents.length();
    while (n_sent < n_total) {
        // wait for socket to be available
        int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_ready < 0) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n_ready; i++) {
            if (events[i].data.fd == sock_fd) {
                // send as much of the file as possible
                int size = std::max(n_total - n_sent, BUF_SIZE);
                char buffer[BUF_SIZE];
                strcpy(buffer, file_contents.substr(n_sent, BUF_SIZE-1).c_str());
                std::cout << "BUFFER: ";
                for (int i = 0; i < size; i++) {
                    std::cout << buffer[i];
                }
                std::cout << std::endl;
                int n_bytes = sendto(sock_fd, buffer, size, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));
                std::cout << "Sending data" << std::endl;
                if (n_bytes < 0) {
                    perror("send failed");
                    exit(EXIT_FAILURE);
                }
                n_sent += n_bytes;
            }
        }
    }
    
//     // End the clock
//     clock_t end_time = clock();
//     double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
//     printf("File transfer speed: %.2f MB/s\n", (double)file_size / elapsed_time / (1024 * 1024));

    // Close the socket
    close(sock_fd);
    return 0;
}
