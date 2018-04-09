/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v4.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>       // read, write, close
#include <string.h>
#include <strings.h>        // bcopy, bzero
#include <netdb.h>        // socket-related structures
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BuffSize 2000

#define ClientLimit 10


int CreateTCPServerSocket(int ServerPort);

int AcceptTCP(int SeverSocket);

void TCPClientMessage(int ClientSocket);



/**************** main() ****************/


int main(const int argc, char *argv[])
{

int *ServerSock_fd;
fd_set SocketSet_fd;
int RunServer = 1;
int NumberPort;
int Port;
unsigned short PortNum;
long TimeOut;
struct timeval SelectTimeOut;
int MaximumDescriptor;

if (argc < 3) {
fprintf(stderr, "\n Usage: ./TCPServer_v4 <TimeOut Seconds>  <Port Number 1>  ... <Port Numbers> ... \n,");
    exit(1);
  } else {
  	TimeOut = atoi(argv[1]);
  	NumberPort = argc - 2;
  	ServerSock_fd = (int *)malloc(NumberPort * sizeof(int));
  }

MaximumDescriptor = -1;
Port = 0;
while (Port < NumberPort) {
	PortNum = atoi(argv[Port + 2]);
	ServerSock_fd[Port] = CreateTCPServerSocket(PortNum);

	if (ServerSock_fd[Port] > MaximumDescriptor) {
		MaximumDescriptor = ServerSock_fd[Port];
	}
	Port++;
}
fprintf(stdout, "Server_v4 Started\n");

while(RunServer) {
	FD_ZERO(&SocketSet_fd);
	FD_SET(STDIN_FILENO, &SocketSet_fd);
	for (Port = 0; Port < NumberPort; Port++) {
		FD_SET(STDIN_FILENO, &SocketSet_fd);
		}
	SelectTimeOut.tv_sec = TimeOut;
	SelectTimeOut.tv_usec = 0;
	if (select(MaximumDescriptor + 1, &SocketSet_fd, NULL, NULL, &SelectTimeOut) == 0) {
		fprintf(stdout, "%ld secs passed, no echo request. Blocking at select().\n", TimeOut);
	} else if (FD_ISSET(0, &SocketSet_fd)) {
		fprintf(stdout, "Closing Server\n");
		getchar();
		RunServer = 0;
	} 
		for (Port = 0; Port < NumberPort; Port++) {
			if (FD_ISSET(ServerSock_fd[Port], &SocketSet_fd)) {
				fprintf(stdout, "New connection, socket fd is ..., IP is : ... , port : %d\n", PortNum);
				TCPClientMessage(AcceptTCP(ServerSock_fd[Port]));
				} 

			}
	}

/*for (Port = 0; Port < NumberPort; Port++) {
	close(ServerSock_fd[Port]);
	}
	*/
free(ServerSock_fd);
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
	sleep(3);
return client_fd;
}



void TCPClientMessage(int client_fd) {

char buffer[BuffSize];      
int receiving;
//sleep(3);

if ((receiving = recv(client_fd, buffer, BuffSize, 0)) < 0) {
	perror("Failed to be received");
}
/* Send received string and receive again until end of transmission */
while (receiving > 0) {
/* Echo message back to client */
if (send(client_fd, buffer, BuffSize, 0) < 0) {
	perror("client failed to send message");

}


if ((receiving = recv(client_fd, buffer, BuffSize, 0)) < 0) {
	//perror("client failed to write message");

	}
	
}

close(client_fd); 
}

