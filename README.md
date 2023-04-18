# UDP client and server for file transfer
This repository implements UDP client and server for file transfer

In order to use the UDP server and client, clone the repository and compile the binaries
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

