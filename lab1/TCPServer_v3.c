/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v3.c
*  This Server program is non-blocking in the sense that it implements threads that echoes messages back to the client
*  concurrently
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

/* constants */
#define BuffSize 2000


/* local function prototypes */

void *ThreadMain(void* arg);

int CreateTCPServerSocket(int ServerPort);

int AcceptTCP(int SeverSocket);


/**************** main() ****************/

int main(const int argc, char *argv[])
{

	int ServerSock;
	int ClientSock;
	unsigned short ServerPort;
	pthread_t ThreadId;


// check arguments
	if (argc != 2) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(1);
	} else {
		ServerPort = atoi(argv[1]);
	}

// arguement checking
	if (isalpha(ServerPort)) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(-1);
	}

// initiate socket for server
	ServerSock = CreateTCPServerSocket(ServerPort);

//starst loop for pthreads
	while (1) {

		for ( ; ; ) {
			// if the client socket descriptor is a positive integer make a thread
			if ((ClientSock = AcceptTCP(ServerSock)) > 0) {
				sleep(3);
				pthread_create(&ThreadId, NULL, ThreadMain, (void*) &ClientSock);
			}
		}
//clean up
		close(ServerSock);

	}
	return 0;
}

/* ThreadMain() ***********************
This is the main thread that is used to connect to the client sockets
Input: for the fuction is the socket descriptor for the client socket
Output: None
********************************************/
void *ThreadMain(void* Socket) {
	int client_fd;


	client_fd = *(int*) Socket;

	char buffer[BuffSize];
	int message;

// create buffer
	bzero(buffer, BuffSize);
// read the client data eg the message to echo back
	message = read(client_fd, buffer, BuffSize);
// error checking
	if (message < 0) {
		perror("ERROR reading from socket");
	}
	// echo back the string
	message = write(client_fd, buffer, strlen(buffer));
	if (message < 0) {
		perror("ERROR writing to socket");
	}

	fprintf(stdout, "Handling with thread: %lu \n", (unsigned long)pthread_self());

// clean up
	close (client_fd);
	pthread_exit(NULL);
}

/********* CreateTCPServerSocket() **************
This function initializes the server socket
Input: integer Server Port number
Output: integer of the server socket descriptor
*************************************************/

int CreateTCPServerSocket(int ServerPort) {

	int server_fd;
	struct sockaddr_in ServerAddress;

// make the server socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("opening socket failed");
		return -1;
	}
//just in case, clear the Server Address structure.
	memset(&ServerAddress, 0, sizeof(ServerAddress));
// define SeverAddress sockaddr structure
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(ServerPort);

	int opt_val = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

// binding
	if (bind(server_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0) {
		perror("bind didn't work");
		return -1;
	}
// listening
	if (listen(server_fd, 5) < 0) {
		perror("listening failed");
		return -1;
	}
	return server_fd;

}

/********* AccteptTCP() **********************
This function connects two sockets, the server socket and a host socket
Input: integer, Server Socket file descriptor
Output: integer, Client socket file descriptor
**********************************************/

int AcceptTCP(int ServerSocket) {

	int client_fd;

	unsigned int ClientLen;

	struct sockaddr_in ClientAddress;

	memset(&ClientAddress, 0, sizeof(ClientAddress));

	ClientLen = sizeof(ClientAddress);
	// accept the client socket and assign client_fd as the client socket descriptor
	if (( client_fd = accept(ServerSocket, (struct sockaddr *) &ClientAddress, &ClientLen)) < 0 ) {
		fprintf(stdout, "accept failed");
		return -1;
		// checking that we actually connected to socket
	} else if (getsockname(client_fd, (struct sockaddr *) &ClientAddress, &ClientLen) == -1) {
		perror("getsockname() failed");
		return -1;
	}

	fprintf(stdout, "Client %s is connected\n", inet_ntoa(ClientAddress.sin_addr));
	return client_fd;
}


