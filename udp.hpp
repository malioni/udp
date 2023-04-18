#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/epoll.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <fcntl.h>

#define MAX_EVENTS 10
#define BUF_SIZE 100
#define TERMINATING_MSG "########END#OF#MESSAGE########"
#define DEST_DIR "server_files/"

// Struct used by server to represent a file
struct transfer_file
{
    std::string name;
    std::stringstream contents;
};

// Function that reads file contents, returns integer whether it was successful or not
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
        return 1;
    }
    
    return 0;
}

// Function that creates a socket
int create_socket()
{
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0)
  {
  perror("socket");
  exit(EXIT_FAILURE);
  }
  
  return sock_fd;
}

// Function that configures IPv4 and port
struct sockaddr_in configure_ip_and_port(int port)
{
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);
  
  return server_addr;
}

// Function that writes a file
int write_file(const transfer_file &f)
{
  std::string file_path = DEST_DIR + f.name.substr(f.name.find_last_of("/") + 1);
  // Open file for writing
  std::ofstream outfile(file_path);

  // Check if file was opened successfully
  if (!outfile.is_open()) {
    perror("Error opening file");
    return 1;
  }

  // Write stringstream contents to file
  outfile << f.contents.rdbuf();

  // Close file
  outfile.close();

  return 0;
}
