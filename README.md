# UDP client and server for file transfer
This repository implements UDP client and server for file transfer

Author: Matiss Senkans

In order to use the UDP server and client, clone the repository and compile the binaries (used C++17 compiler)
```
g++ server.cpp -o server
g++ client.cpp -o client
```
The server requires ports to be specified as command-line arguments. They have to be space separated, it can either be a single port or multiple, as in:
```
./server 49000
```
or
```
./server 49000 8080
```
Note that the server saves files in a `./server_files` directory, which has to be created beforehand by running the following command in the same directory as the server.cpp file:
```
mkdir server_files
```

The client requires the port number and the file to be specified as command-line arguments. For example:
```
./client 49000 client_files/test.txt
```

# Notes
Implemented file transfer protocol is as follows:
1) The client sends the file name as it's first message, which the server receives and saves in memory. File name should not exceed 1024 characters, which is UDP data transfer limit.
2) The client then sends the file contents in 1024 byte increments, which the server receives and combines.
3) After all file contents have been sent, the client sends a terminating message (currently defined as "########END#OF#MESSAGE########") to indicate that all file contents have been sent. Note that having this message as part of a file could potentially break the program.
4) When the server receives the terminating message, it sends the same message back to the client to indicate that it has been received. 
