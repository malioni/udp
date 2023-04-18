#include <chrono>
#include "udp.hpp"
   
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    std::string file_name(argv[2]);
    
    std::string file_contents;
    // read the file
    if (read_file(file_name, file_contents) != 0)
    {
        perror("Error reading the file");
        exit(EXIT_FAILURE);
    }

    // create a UDP socket
    int sock_fd = create_socket();

    // Configure IPv4 and correct port
    struct sockaddr_in server_addr = configure_ip_and_port(port);

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
   
    // start measuring time
    std::cout << "Sending the file name" << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    auto total_start_time = start_time;

    // Send the file name
    int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    int n_bytes;
    if (n_ready < 0) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < n_ready; i++) {
        if (events[i].data.fd == sock_fd) {
            // send the file name
            int size = file_name.length();
            char buffer[size];
            strcpy(buffer, file_name.c_str());
            int n_bytes = sendto(sock_fd, buffer, size, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));
            if (n_bytes < 0) {
                perror("send failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    auto end_time = std::chrono::steady_clock::now();
    std::cout << "File name sent" << std::endl;
    double elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0;
    double transfer_speed = (double)(n_bytes) / (elapsed_time * 1024 * 1024);
    std::cout << "Transfer speed: " << transfer_speed / 1000000 << " MB/s" << std::endl;
    start_time = end_time;

    // Send the file contents
    int n_sent = 0;
    int n_total = file_contents.length();

    while (n_sent < n_total) {
        // wait for socket to be available
        n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_ready < 0) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < n_ready; i++) {
            if (events[i].data.fd == sock_fd) {
                // send as much of the file as possible
                int size = std::min(n_total - n_sent, BUF_SIZE);
                char buffer[size];
                strcpy(buffer, file_contents.substr(n_sent, size).c_str());
                std::cout << "Sending a message" << std::endl;
                n_bytes = sendto(sock_fd, buffer, size, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));
                if (n_bytes < 0) {
                    perror("send failed");
                    exit(EXIT_FAILURE);
                }
                n_sent += n_bytes;
                std::cout << "Message sent" << std::endl;
                end_time = std::chrono::steady_clock::now();
                elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0;
                transfer_speed = (double)(n_bytes) / (elapsed_time * 1024 * 1024);
                std::cout << "Transfer speed: " << transfer_speed / 1000000 << " MB/s" << std::endl;
                start_time = end_time;
            }
        }
    }
    std::cout << "Sending the terminating msg" << std::endl;
    // Send the terminating message
    std::string msg(TERMINATING_MSG);
    n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (n_ready < 0) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < n_ready; i++) {
        if (events[i].data.fd == sock_fd) {
            // send the file name
            int size = msg.length();
            char buffer[size];
            strcpy(buffer, msg.c_str());
            n_bytes = sendto(sock_fd, buffer, size, 0,(struct sockaddr *)&server_addr, sizeof(server_addr));
            if (n_bytes < 0) {
                perror("send failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    std::cout << "Terminating msg sent" << std::endl;
    end_time = std::chrono::steady_clock::now();
    elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0;
    transfer_speed = (double)(n_bytes) / (elapsed_time * 1024 * 1024);
    std::cout << "Transfer speed: " << transfer_speed / 1000000 << " MB/s" << std::endl;
    
    elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - total_start_time).count() / 1000000.0;
    transfer_speed = (double)(n_total) / (elapsed_time * 1024 * 1024);
    std::cout << "Total file transfer speed: " << transfer_speed / 1000000 << " MB/s" << std::endl;
    std::cout << "Total time: " << elapsed_time << std::endl;
    
    std::cout << "Waiting for response" << std::endl;
    // Wait for response
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event) < 0) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    
    n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (n_ready < 0) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < n_ready; i++) {
        if (events[i].data.fd == sock_fd) {
            char buffer[BUF_SIZE];
            socklen_t server_addr_len = sizeof(server_addr);
            int n_bytes = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &server_addr_len);
            if (n_bytes < 0) {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
        }
    }
    std::cout << "Response received" << std::endl;
    
    
//     // TODO: End the clock
//     clock_t end_time = clock();
//     double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
//     printf("File transfer speed: %.2f MB/s\n", (double)file_size / elapsed_time / (1024 * 1024));

    // Close the socket
    close(sock_fd);
    return 0;
}
