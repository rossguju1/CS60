#CS60 Lab1 README.md
Ross Guju

README for lab1. Files include: TCPClient.c, TCPServer_v1.c, TCPServer_v2.c, TCPServer_v3.c, TCPServer_v4.c, and Makefile

## Compiling/Make
The Makefile compiles all of the c files with the following gcc flags:
gcc -Wall -pedantic -std=c11 -ggdb, this is the equivalent to `mygcc` which was our alias for compiling all labs and projects. 

However, TCPServer_v3.c need an extra flag `-lpthread` to include the pthread.h library.

To build all of the files, run `make all` 

To clean up, run `make clean`

### Program Usages

TCPClient.c - The client program for the echo server/client models. This program attempts to connect with the portnumber of the input hostname. Frorm there it sends a string and waits until the server echoes the same string. 

Input: ./TCPClient <Target Server Addr> <Port Num> <Message String> 
Output: Status of the program. Non- negative implies success.

stdin: 
commandline Arguments: ./TCPClient flume.cs.dartmouth.edu  8889 "request 1"

stdout:
Received: request 1

stderr:
prints error status/report to stderr.


TCPServer_v1.c - 

TCPServer_v2.c - 

TCPServer_v3.c - 

TCPServer_v4.c - 

#### Example Command Lines (tests)



TEST 1:


###### Assumptions 



