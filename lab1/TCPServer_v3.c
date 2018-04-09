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
#include <pthread.h>

#define BuffSize 2000

void *ThreadMain(void* arg);
int CreateTCPServerSocket(int ServerPort);

int AcceptTCP(int SeverSocket);

void TCPClientMessage(int ClientSocket);

struct ThreadArgs {
int ClientSock;
};

/**************** main() ****************/
int main(const int argc, char *argv[])
{
int ServerSock;
int ClientSock;
unsigned short ServerPort;
pthread_t ThreadId;
struct ThreadArgs *threadargs;
int server_fd, client_fd;


if (argc != 2) {
fprintf(stderr, "\n usage: ./TCPServer_v3  <Port Number> \n,");
    exit(1);
  } else {
  	ServerPort = atoi(argv[1]);
  }

 ServerSock = CreateTCPServerSocket(ServerPort);

 while(1) {
 	ClientSock = AcceptTCP(ServerSock);
 	if ((threadargs = (struct  ThreadArgs*) malloc(sizeof( struct  ThreadArgs ))) == NULL) {
 		perror("failed to malloc memory");
 		exit(-1);
 	}
 	threadargs->ClientSock = ClientSock;
 	 
 	/* if (pthread_create(&ThreadId, NULL, ThreadMain, (void*) ThreadArgs) != 0 ) {
 		perror("thread failed to be made");
 		exit(-1);
 	} */
 	//else {
 	    pthread_create(&ThreadId, NULL, ThreadMain, (void*) threadargs);
 	    sleep(1);
 		fprintf(stdout, "\n~~~~~Handling with thread: %d ~~~~~\n", pthread_self());
		printf("\n~~~~~PID of this process: %d ~~~~~\n", getpid());
		printf("\n~~The ID of this thread is: %u\n", (unsigned int)pthread_self());
 		sleep(1);
 		//fprintf(stdout, "Handling with thread: %d \n", ThreadId);
 	//}

 }
return 0;
}

void *ThreadMain(void* threadargs) {
int ClientSock;
fprintf(stdout, "Handling with thread: %d \n", pthread_self());
printf("PID of this process: %d\n", getpid());
printf("The ID of this thread is: %u\n", (unsigned int)pthread_self());
sleep(1);
pthread_detach(pthread_self());
sleep(1);
ClientSock = ((struct  ThreadArgs*) threadargs)->ClientSock;
free(threadargs);
TCPClientMessage(ClientSock);
return NULL;
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



