#CS60 Lab1 README.md
Ross Guju

README for lab1. Files include: TCPClient.c, TCPServer_v1.c, TCPServer_v2.c, TCPServer_v3.c, TCPServer_v4.c, and Makefile

## Compiling/Make
The Makefile compiles all of the c files with the following gcc flags:
gcc -Wall -pedantic -std=c11 -ggdb, this is the equivalent to `mygcc` which was our alias for compiling all labs and projects. 

However, TCPServer_v3.c needed an extra flag `-lpthread` to include the pthread.h library.

To build all of the files, run `make` 

To clean up, run `make clean`

### Implementation 
`TCPServer_v4.c` - My implementation of TCPServer_v4.c was the intention of matching the behavior of Professor's Xhou implementation: 
(This code was shared on Piazza)

./TCPServer_v4 10 8888
Listen on port 8888
Waiting for connections ...
10 secs passed, no echo request. Blocking at select().
10 secs passed, no echo request. Blocking at select().
New connection, socket fd is 4, IP is : 127.0.0.1, port : 51731
New connection, socket fd is 4, IP is : 127.0.0.1, port : 51733

To accomplish this fuctionality when using the `select()` function, I proceded the same method as I did in the previous tasks to get the server socket to start listening for connections from the client. 

when I got to the first `while()` for accepting the client sockets, I added the `MasterSocket_fd` descriptor to `FD_SET()`. Since `FD_SET()` is a data structure that contains the socket descriptors that I will be running a process over, it is important to keep track of the number of elements in `FD_SET()` because it is much faster to only loop through the elements I need. 

However, before actually calling the `select()` function, I initialize the TimeOut time and pass the timeval structure as its input so that `select()` knows how long to wait until one of the socket descriptors is ready to be read.

So once there is an incoming message from the client, we assign it a descriptor that is both readable and writable and then add it to the descriptor set. 

Now that we have the set of descriptors and the number of descriptors in the set ie `max_fd`, we loop through the set reading the message string and then writing it back to the client.

### Program Usages

`TCPClient.c` - The client program for the echo server/client models. This program attempts to connect with the portnumber of the input hostname. Frorm there it sends a string and waits until the server echoes the same string. 

Input: ./TCPClient <Target Server Addr> <Port Num> <Message String> 
Output: Status of the program. Non-negative implies success.

stdin: 
commandline Arguments: ./TCPClient flume.cs.dartmouth.edu  8889 "request 1"

stdout:
Received: request 1

stderr:
prints error status/report to stderr.


`TCPServer_v1.c` - Server version one is a simple I/O model that starts up a blocking socket which then is accepted by TCPClient.

Input: ./TCPServer_v1 <Port Number>
Output: Status of the program. Non-negative implies success.

stdin: 
commandline Arguments: ./TCPServer_v1 8888

stdout:
Client 129.170.214.115 is connected 

stderr:
prints error status/report to stderr.

`TCPServer_v2.c` - This is implementation of the server has the same fuctionality, however, uses the fork() function to create new processes.
Input: ./TCPServer_v2 <Port Number>
Output: Status of the program. Non-negative implies success.

stdin: 
commandline Arguments: ./TCPServer_v2 8890

stdout:
Client 129.170.214.115 is connected
Handling with child process: 7302

stderr:
prints error status/report to stderr.

`TCPServer_v3.c` - This implementation of the echoing server uses pthreads to allow multiple sockets to connect to the server.

Input: ./TCPServer_v3 <Port Number>
Output: Status of the program. Non-negative implies success.

stdin: 
commandline Arguments: ./TCPServer_v3 8888

stdout:
Client 129.170.214.115 is connected
Handling with child process: 7302

stderr:
prints error status/report to stderr.

`TCPServer_v4.c` - This implementation uses multiplexing to create non-blocking I/O model of the server side for echoing a message from a client.

Input: ./TCPServer_v4 <TimeOut (Seconds)> <Port Number>
Output: Status of the program. Non-negative implies success.

stdin: 
commandline Arguments: ./TCPServer_v4 5 8888

stdout:
Listening on Port 8888
Waiting for connections......
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
New connection, socket fd is 4, IP is : 10.31.209.163, PORT : 52070 

stderr:
prints error status/report to stderr.

#### Example Command Lines (tests)

TEST 1:

[flume:lab1] 97) ./TCPServer_v1 8888
Client 129.170.214.115 is connected
Client 129.170.214.115 is connected
Client 129.170.214.115 is connected

Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 1"
Received: request 1 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 2"
Received: request 2 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 3"
Received: request 3 
Rosss-MacBook-Air:lab1 rossguju$

TEST 2:

[flume:lab1] 98) ./TCPServer_v2 8888
Client 129.170.214.115 is connected
Handling with child process: 7302
Client 129.170.214.115 is connected
Handling with child process: 7304
Client 129.170.214.115 is connected
Handling with child process: 7307
Client 129.170.214.115 is connected
Handling with child process: 7308
Client 129.170.214.115 is connected
Handling with child process: 7310

Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 10"
Received: request 10 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 11"
Received: request 11 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 12"
Received: request 12 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 13"
Received: request 13 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 14"
Received: request 14 

TEST 3:

[flume:lab1] 102) ./TCPServer_v3 8888
Client 129.170.214.115 is connected
Handling with thread: 140709561636608 
Client 129.170.214.115 is connected
Handling with thread: 140709551052544 
Client 129.170.214.115 is connected
Handling with thread: 140709542659840 
Client 129.170.214.115 is connected
Handling with thread: 140709534267136


Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 20"
Received: request 20 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 21"
Received: request 21 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 22"
Received: request 22 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 23"
Received: request 23 
Rosss-MacBook-Air:lab1 rossguju$ 

TEST 4:

[flume:lab1] 102) ./TCPServer_v4 5 8888
Listening on Port 8888
Waiting for connections......
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().
New connection, socket fd is 4, IP is : 10.31.209.163, PORT : 52070 
5 secs passed, no echo request. Blocking at select().
New connection, socket fd is 4, IP is : 10.31.209.163, PORT : 52071 
New connection, socket fd is 4, IP is : 10.31.209.163, PORT : 52072 
New connection, socket fd is 4, IP is : 10.31.209.163, PORT : 52073 
5 secs passed, no echo request. Blocking at select().
5 secs passed, no echo request. Blocking at select().

Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 30"
Received: request 30 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 31"
Received: request 31 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 32"
Received: request 32 
Rosss-MacBook-Air:lab1 rossguju$ ./TCPClient flume.cs.dartmouth.edu  8888 "request 33"
Received: request 33 


###### Assumptions 

1. The server is ran on flume.cs.dartmouth.edu
2. The client ran is on a local device.

