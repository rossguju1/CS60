/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPClient.c
*
* This program is the client program that sends a message to one of my server programs and the server echo's back
* the message to the client. 
* ./TCPClient is used for all of the TCPServer programs
*/


#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>        
#define BuffSize 2000

/**************** main() ****************/

int main(const int argc, char *argv[])
{
char *hostname;
int Port = 0;
char *message;
int socket_fp;
struct sockaddr_in ServerAddress;


// check arguments
if (argc != 4) {
	fprintf(stderr, "\n Usage: %s <Target Server Addr> <Port Num> <Message String> \n", argv[0]);
} else {
hostname = argv[1]; // server address ie flume.cs.dartmouth.edu
Port = atoi(argv[2]); // the server port
message = argv[3]; // message to be echoed
}

  // arguement checking
  if (isalpha(Port)) {
  fprintf(stderr, "\n Usage: %s <Target Server Addr> <Port Num> <Message String> \n", argv[0]);
    exit(-1);
  }


struct hostent *hostp = gethostbyname(hostname);
if (hostp == NULL) {
    fprintf(stderr, "startup: unknown host name '%s'\n", hostname);
    exit(-1);
  }

memset(&ServerAddress, 0, sizeof(ServerAddress)); // clear ServerAddress variable

//Initialize sockaddr_in for Server
ServerAddress.sin_family = AF_INET;
bcopy(hostp->h_addr_list[0], &ServerAddress.sin_addr, hostp->h_length);
ServerAddress.sin_port = htons(Port);

  // Create socket
if ((socket_fp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   	perror("Failed to open socket");
   	exit(-1); 
}
// Connect to server 
if (connect(socket_fp, (struct sockaddr*) &ServerAddress, sizeof(ServerAddress)) < 0) {
    perror("Failed to connect to server");
   	exit(-1); 
  } else {

    // send server the message
  send(socket_fp, message, strlen(message), 0);

  // make the buffer to insert the echoed response from the Sercer
  char RecieveMessage[BuffSize];
  // make the buffer
  memset(RecieveMessage, 0, BuffSize*sizeof(char));

  // read the server's message 
  read(socket_fp, RecieveMessage, BuffSize);
  //write the message into the buffer
  write(socket_fp, RecieveMessage, strlen(RecieveMessage + 1));
  printf("Received: %s \n", RecieveMessage);


  }
  // clean up
  close(socket_fp);
  return 0;
}


