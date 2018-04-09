/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v3.c
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




/**************** main() ****************/
int main(const int argc, char *argv[])
{
int ServerSock;
int ClientSock;
unsigned short ServerPort;
pthread_t ThreadId;




if (argc != 2) {
fprintf(stderr, "\n usage: ./TCPServer_v3  <Port Number> \n,");
    exit(1);
  } else {
  	ServerPort = atoi(argv[1]);
  }


 ServerSock = CreateTCPServerSocket(ServerPort);


 while (1) { 

 	for ( ; ; ) {
 	if ((ClientSock = AcceptTCP(ServerSock)) > 0) {
 		sleep(3);
 	    pthread_create(&ThreadId, NULL, ThreadMain, (void*) &ClientSock);
 	}
 }

close(ServerSock);
 
}
return 0;
}

void *ThreadMain(void* Socket) {
int client_fd;


client_fd = *(int*) Socket;

char buffer[BuffSize];      
int message;

bzero(buffer, BuffSize);
message = read(client_fd, buffer, BuffSize);
if (message < 0) {
	  perror("ERROR reading from socket");
      }
    message = write(client_fd, buffer, strlen(buffer));
      if (message < 0) {
	     perror("ERROR writing to socket");
      }
     
fprintf(stdout, "Handling with thread: %lu \n", (unsigned long)pthread_self());


close (client_fd);
pthread_exit(NULL);
}

int CreateTCPServerSocket(int ServerPort) {

int server_fd;
struct sockaddr_in ServerAddress;

if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
 	 perror("opening socket failed");
 	 return -1;
 }

memset(&ServerAddress, 0, sizeof(ServerAddress));

ServerAddress.sin_family = AF_INET;
ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
ServerAddress.sin_port = htons(ServerPort); 

int opt_val = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
	

if (bind(server_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0){
	perror("bind didn't work");
	return -1;
}

 if (listen(server_fd, 5) < 0) {
 	perror("listening failed");
 	return -1;
}	
return server_fd;

}

int AcceptTCP(int ServerSocket) {

int client_fd;

unsigned int ClientLen;   

struct sockaddr_in ClientAddress;

memset(&ClientAddress, 0, sizeof(ClientAddress));

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

