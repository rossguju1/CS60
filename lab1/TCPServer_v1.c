/* Ross Guju
*  CS60 Networks
*  Lab1
*  TCPServer_v1.c
*  This program creates a server on flume.cs.dartmouth.edu and listens for my TCPClient program to send a
*  string where the sever will echo it back to the client.
*  This Server implementation uses blocking sockets and only can make a socket with one host at a time.
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

/******** Constants ***********/
#define BuffSize 2000

/**************** main() ****************/
int main(const int argc, char *argv[])
{

	int receiving;
	struct sockaddr_in ServerAddress, ClientAddress;
	char buffer[BuffSize];  // buffer for receiving message
	unsigned short ServerPort;
	unsigned int ClientLen;
	int server_fd, client_fd;


// check arguments
	if (argc != 2) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(-1);
	} else {
		ServerPort = atoi(argv[1]);
	}

// arguement checking
	if (isalpha(ServerPort)) {
		fprintf(stderr, "usage: %s <Port Number>\n", argv[0]);
		exit(-1);
	}

// make the socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("opening socket failed");
		exit(-1);
	}
//Initialize sockaddr_in for Server
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(ServerPort);


// setsockopt() makes sure that the server socket can connect with multiple hosts
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
// enter into continuous loop tp initiate the server to listen
	while (1) {

		ClientLen = sizeof(ClientAddress);
		//if sever connected with the client, then client_fd is the host file descriptor of its socket
		if (( client_fd = accept(server_fd, (struct sockaddr *) &ClientAddress, &ClientLen)) < 0 ) {
			fprintf(stdout, "accept failed");
			exit(-1);
			// get information on the client
		} else if (getsockname(client_fd, (struct sockaddr *) &ClientAddress, &ClientLen) == -1) {
			perror("getsockname() failed");
			exit(-1);
		}
		fprintf(stdout, "Client %s is connected\n", inet_ntoa(ClientAddress.sin_addr));

		// entire another loop to receive and echo the clients message
		while (1) {
			// if nothing was recieved
			memset(buffer, 0, BuffSize*sizeof(char));
			if ((receiving = recv(client_fd, buffer, BuffSize, 0)) == 0) {
				break;
			} else if (receiving < 0) {
				perror("client was unable to read");
			}
			//echo the message back to the client
			if (send(client_fd, buffer, receiving, 0) < 0) {
				perror("client failed to write message");
			}
			break;
		}
	}
	return 0;
}


