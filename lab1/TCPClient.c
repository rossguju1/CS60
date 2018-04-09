#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>        // socket-related structures

/**************** main() ****************/
int main(const int argc, char *argv[])
{
char *hostname;
int Port;
char *message;
int socket_fp;
struct sockaddr_in ServerAddress;


if (argc != 4) {
	fprintf(stderr, "\n Usage: ./TCPClient <Target Server Addr> <Port Num> <Message String> \n");
} else {
hostname = argv[1];
Port = atoi(argv[2]);
message = argv[3];
}

struct hostent *hostp = gethostbyname(hostname);
if (hostp == NULL) {
    fprintf(stderr, "startup: unknown host '%s'\n", hostname);
    exit(-1);
  }

memset(&ServerAddress, 0, sizeof(ServerAddress));
ServerAddress.sin_family = AF_INET;
bcopy(hostp->h_addr_list[0], &ServerAddress.sin_addr, hostp->h_length);
ServerAddress.sin_port = htons(Port);

  // Create socket
if ((socket_fp = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   	perror("Failed to open socket");
   	exit(-1); 
}
if (connect(socket_fp, (struct sockaddr*) &ServerAddress, sizeof(ServerAddress)) < 0) {
    perror("Failed to connect to server");
   	exit(-1); 
  } else {

  send(socket_fp, message, strlen(message), 0);
  int n = 0;
  int len = 0, maxlen = 100;
  char buffer[maxlen];
  char* pbuffer = buffer;

  // will remain open until the server terminates the connection
  while ((n = recv(socket_fp, pbuffer, maxlen, 0)) > 0) {
    pbuffer += n;
    maxlen -= n;
    len += n;

    buffer[len] = '\0';
    printf("Received: %s", buffer);
  	}
  }

  close(socket_fp);
  return 0;
  }








