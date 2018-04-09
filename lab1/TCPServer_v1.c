/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v1.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>       // read, write, close
#include <string.h>
#include <strings.h>        // bcopy, bzero
#include <netdb.h>        // socket-related structures
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BuffSize 2000


/**************** main() ****************/
int main(const int argc, char *argv[])
{

int receiving;
struct sockaddr_in ServerAddress, ClientAddress;
char buffer[BuffSize];
char *hostname = "flume.cs.dartmouth.edu";       // server hostname


unsigned short ServerPort;
unsigned int ClientLen;         //server port
int server_fd, client_fd;



if (argc != 2) {
fprintf(stderr, "\n usage: ./TCPServer_v1  <Port Number> \n,");
    exit(1);
  } else {
  	ServerPort = atoi(argv[1]);
  }	


if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
 	 perror("opening socket failed");
     return -1;
 }

//memset(&ServerAddress, 0, sizeof(ServerAddress));

ServerAddress.sin_family = AF_INET;
ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
ServerAddress.sin_port = htons(ServerPort); 

int opt_val = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
	

if (bind(server_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0){
	perror("bind didn't work");
	return -1;
}

 if (listen(server_fd, 3) < 0) {
 	perror("listening failed");
 	return -1;
}

while(1) {
	ClientLen = sizeof(ClientAddress);
	if (( client_fd = accept(server_fd, (struct sockaddr *) &ClientAddress, &ClientLen)) < 0 ) {
		fprintf(stdout, "accept failed");
		exit(-1);
	} else if (getsockname(client_fd, (struct sockaddr *) &ClientAddress, &ClientLen) == -1) {
		perror("getsockname() failed");
		exit(-1);
	}
	fprintf(stdout, "Client %s is connected\n", inet_ntoa(ClientAddress.sin_addr));
	
	while(1) {
		if ((receiving = recv(client_fd, buffer, BuffSize, 0)) == 0) {
			//fprintf(stdout, "Client %s is connected", inet_ntoa(ClientAddress.sin_addr));

				break; 
			} else if (receiving < 0) {
			perror("client was unable to read");
			}
		if (send(client_fd, buffer, receiving, 0) < 0) {
			perror("client failed to write message");
		}
		//fprintf(stdout, "Client %s is connected", inet_ntoa(ClientAddress.sin_addr));
		break;
	}


	}
	//fprintf(stdout, "Client %s is connected", inet_ntoa(ClientAddress.sin_addr));
return 0;
}














