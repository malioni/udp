#include <map>
#include <algorithm>
#include "udp.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <ports>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Register the signal handler for SIGINT
    signal(SIGINT, sigint_handler);
    
    std::map<std::string, transfer_file> m_sock_to_file;
    std::stringstream ss;
    std::vector<int> ports;
    std::vector<int> sockets;
    
    for (int i = 1; i < argc; i++)
    {
        ports.push_back(atoi(argv[i]));
    }
    
    int ports_size = ports.size();
    
    // Create sockets
    for (int i = 0; i < ports_size; i++)
    {
        sockets.push_back(create_socket());
    }
    
    for (int i = 0; i < ports_size; i++)
    {
        // Configure IPv4 and port
        struct sockaddr_in server_addr = configure_ip_and_port(ports.at(i));
        
        // Bind socket's name to address
        if (bind(sockets.at(i), (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        
        // Set socket to non-blocking so that one socket does not block the others
        fcntl(sockets.at(i), F_SETFL, O_NONBLOCK);
        
    }
    
    // Create epoll event and field descriptor
    struct epoll_event event, events[MAX_EVENTS];
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Specify that events are incoming to the server
    event.events = EPOLLIN;
    for (int i = 0; i < ports_size; i++)
    {
        event.data.fd = sockets.at(i);
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets.at(i), &event) == -1)
        {
            perror("epoll_ctl");
            exit(EXIT_FAILURE);
        }
    }

    while (!flag) {
        // Wait for events to happen
        int n_ready = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_ready < 0) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n_ready; i++) {
            // Check that events occurred on on of the sockets
            if ( std::find(sockets.begin(), sockets.end(), events[i].data.fd) != sockets.end() )
            {
                char buffer[BUF_SIZE];
                memset(&buffer[0], 0, sizeof(buffer));
                int sock_fd = events[i].data.fd;
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                
                // Receive the information
                int n_bytes = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
                if (n_bytes < 0) 
                {
                    perror("recvfrom");
                    exit(EXIT_FAILURE);
                }
                
                // Convert IP address and port to string to be used as an identifier of the client
                ss.str(std::string());
                ss << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);
                
                // if a new client, the first message is the file name
                if (m_sock_to_file.find(ss.str()) == m_sock_to_file.end())
                {
                    std::cout << "File name received from: " << ss.str() << std::endl;
                    m_sock_to_file[ss.str()].name = buffer;
                }
                else
                {
                    // if an existing client, check whether the message is the terminating message
                    std::string msg(buffer);
                    if (msg == TERMINATING_MSG)
                    {
                        std::cout << "Terminating message received from: " << ss.str() << std::endl;
                        // Send a response
                        event.events = EPOLLIN | EPOLLOUT;
                        event.data.fd = sock_fd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event) == -1)
                        {
                            perror("epoll_ctl mod");
                            exit(EXIT_FAILURE);
                        }
                        char response[] = TERMINATING_MSG;
                        int n_bytes = sendto(sock_fd, response, strlen(response), 0,(struct sockaddr *)&client_addr, client_addr_len);
                        if (n_bytes < 0) {
                            perror("response failed");
                            exit(EXIT_FAILURE);
                        }
                        std::cout << "Response sent to: " << ss.str() << std::endl;
                        // Revert back to inputs only
                        event.events = EPOLLIN;
                        event.data.fd = sock_fd;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event) == -1)
                        {
                            perror("epoll_ctl mod");
                            exit(EXIT_FAILURE);
                        }
                        
                        // Write file
                        std::cout << "Writing the file received from: " << ss.str() << std::endl;
                        int write = write_file(m_sock_to_file[ss.str()]);
                        if (write != 0)
                        {
                            perror("write_file failed");
                            exit(EXIT_FAILURE);
                        }
                        
                        // Delete client from map
                        m_sock_to_file.erase(ss.str());
                    }
                    else
                    {
                        // Add the message to the contents of the file
                        std::cout << "Received some of the file contents from: " << ss.str() << std::endl;
                        m_sock_to_file[ss.str()].contents << buffer;
                    }
                }
            }
        }
    }

    // Close all ports
    for (int i = 0; i < ports_size; i++)
    {
        close(sockets.at(i));
    }
    
    close(epoll_fd);
    return 0;
}
