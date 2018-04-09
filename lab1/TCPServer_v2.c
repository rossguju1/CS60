/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v2.c
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
#include <sys/wait.h>


#define BuffSize 2000


int CreateTCPServerSocket(int ServerPort);

int AcceptTCP(int SeverSocket);

void TCPClientMessage(int ClientSocket);



/**************** main() ****************/
int main(const int argc, char *argv[])
{

pid_t ProcessID;
unsigned int ChildCounter;

int ServerSock, ClientSock;
char *hostname = "flume.cs.dartmouth.edu";       // server hostname
unsigned short Port;


if (argc != 2) {
fprintf(stderr, "\n usage: ./TCPServer_v1  <Port Number> \n,");
    exit(-1);
  } else {
  	Port = atoi(argv[1]);
  }	

ServerSock = CreateTCPServerSocket(Port);

ChildCounter = 0;

while(1) {
	ClientSock = AcceptTCP(ServerSock);

if ((ProcessID = fork()) < 0 ) {
	perror("forking process failed");
	exit(-1);
} else if (ProcessID == 0) {
	close(ServerSock);
	TCPClientMessage(ClientSock);
	exit(-1);
}
close(ClientSock);
ChildCounter++;
while(ChildCounter > 0) {
	ProcessID = waitpid((pid_t) -1, NULL, WNOHANG);
	printf("Handling with child process: %d\n", ProcessID);
	if (ProcessID < 0) {
		perror("Something went wrong");
	} else if ( ProcessID == 0) {
		break;
	} else {
		ChildCounter--;
	}
}

	}
	//fprintf(stdout, "Client %s is connected", inet_ntoa(ClientAddress.sin_addr));
return 0;
}


int CreateTCPServerSocket(int ServerPort) {

int server_fd;

struct sockaddr_in ServerAddress;

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
return server_fd;

}

int AcceptTCP(int ServerSocket) {

int client_fd;

unsigned int ClientLen;   

struct sockaddr_in ClientAddress;

ClientLen = sizeof(ClientAddress);

	if (( client_fd = accept(ServerSocket, (struct sockaddr *) &ClientAddress, &ClientLen)) < 0 ) {
		fprintf(stdout, "accept failed");
		return -1;
	} else if (getsockname(client_fd, (struct sockaddr *) &ClientAddress, &ClientLen) == -1) {
		perror("getsockname() failed");
		return -1;
	}
	fprintf(stdout, "Client %s is connected\n", inet_ntoa(ClientAddress.sin_addr));
return client_fd;
}



void TCPClientMessage(int client_fd) {

char buffer[BuffSize];      
int receiving;

if ((receiving = recv(client_fd, buffer, BuffSize, 0)) < 0) {
	perror("Failed to be received");
}
/* Send received string and receive again until end of transmission */
while (receiving > 0) {
/* Echo message back to client */
if (send(client_fd, buffer, BuffSize, 0) < 0) {
	perror("client failed to send message");

}

/* See if there is more data to receive */
//if ((receiving = recv(client_fd, buffer, BuffSize, 0)) < 0) {
	//perror("client failed to write message");

	//}
}

close(client_fd); 
}





