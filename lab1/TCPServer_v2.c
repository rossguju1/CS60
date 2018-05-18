/* Ross Guju
*  CS60 Networks
*
*  Lab1 - Socket Programming
*
*  TCPServer_v2.c
*
*  This Server program initiates a server that creates child processes by the fork() function when the TCPClient
*  program communicates with it.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>


/* constants */
#define BuffSize 2000


/* local function prototypes */

int CreateTCPServerSocket(int ServerPort);

int AcceptTCP(int SeverSocket);

//void TCPClientMessage(int ClientSocket);



/**************** main() ****************/

int main(const int argc, char *argv[])
{

	pid_t ProcessID;
	unsigned int ChildCounter;
	int ServerSock, ClientSock;
	unsigned short Port;
	char buffer[BuffSize];
	int message;

// arguement checking
	if (argc != 2) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(-1);
	} else {
		Port = atoi(argv[1]);
	}

// arguement checking
	if (isalpha(Port)) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(-1);
	}

// create server socket
	ServerSock = CreateTCPServerSocket(Port);

// initiate the child process counter
	ChildCounter = 0;

	while (1) {
		// make the connection
		ClientSock = AcceptTCP(ServerSock);

//check the return value of fork()
		ProcessID = fork();
		if (ProcessID == 0) {
			close(ServerSock);


// create buffer
	memset(buffer, 0, BuffSize*sizeof(char));
// read the client data eg the message to echo back
			message = read(ClientSock, buffer, BuffSize);
// error checking
			if (message < 0) {
				perror("ERROR reading from socket");
			}
			// echo back the string
			message = write(ClientSock, buffer, strlen(buffer));
			if (message < 0) {
				perror("ERROR writing to socket");
			}

			close(ClientSock);
			exit(0);
		}

		printf("Handling with child process: %d\n", (int) ProcessID);
		close(ClientSock);
		ChildCounter++;
		while (ChildCounter) {
			ProcessID = waitpid((pid_t) - 1, NULL, WNOHANG);
			if (ProcessID < 0) {
				fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
			}
			else if (ProcessID == 0) {
				break;
			} else {
				ChildCounter--;
			}
		}
	}
	return 0;
}

/**************** CreateTCPServerSocket() **************
This function initializes the server socket
Input: integer Server Port number
Output: integer of the server socket descriptor
*************************************************/

int CreateTCPServerSocket(int ServerPort) {

	int server_fd;

	struct sockaddr_in ServerAddress;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("opening socket failed");
		return -1;
	}

// initialize SeverAddress sockaddr structure
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(ServerPort);

	int opt_val = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));


	if (bind(server_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0) {
		perror("bind didn't work");
		return -1;
	}

	if (listen(server_fd, 3) < 0) {
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


