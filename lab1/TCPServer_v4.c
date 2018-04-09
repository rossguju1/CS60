/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v4.c
*  Non-blocking I/O model for a server/client echoing program. This program uses select() function to concurrently connect to
*  client sockets
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

/* constants */

#define BuffSize 2000


/**************** main() ****************/

int main(int argc, char **argv)
{
  int MasterSocket_fd;
  int Socket_fd, max_fd;
  unsigned short PortNumber;
  unsigned int clientlen;
  struct sockaddr_in ServerAddress;
  struct sockaddr_in ClientAddress;
  char buffer[BuffSize];
  int opt_val;
  int message;
  int connection;
  int ServerRunning;
  fd_set SelectSock;
  long TimeOut;
  struct timeval SelectTimeOut;
  int i;

  // argument checking
  if (argc != 3) {
    fprintf(stderr, "usage: %s <TimeOut (Seconds)> <Port Number>\n", argv[0]);
    exit(-1);
  }
  // initialize portnumber and Time out for select()
  PortNumber = atoi(argv[2]);
  TimeOut = atoi(argv[1]);
  // arguement checking
  if (isalpha(PortNumber) || isalpha(TimeOut)) {
    fprintf(stderr, "usage: %s <TimeOut (Seconds)> <Port Number>\n", argv[0]);
    exit(-1);
  }
// initialize the main socket, which is the server socket
  MasterSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (MasterSocket_fd < 0) {
    perror("opening socket failed");
    exit(-1);
  }


  opt_val = 1;
  setsockopt(MasterSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));


  bzero((char *) &ServerAddress, sizeof(ServerAddress));
  // get address information for the server
  ServerAddress.sin_family = AF_INET;

  ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  ServerAddress.sin_port = htons(PortNumber);

  // binding
  if (bind(MasterSocket_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0) {
    perror("Error when Binding");
    exit(-1);
  }

//listening
  if (listen(MasterSocket_fd, 3) < 0) {
    perror("Error when listening");
    exit(-1);
  }
  printf("Listening on Port %d\n", PortNumber);
  printf("Waiting for connections......\n");

  ServerRunning = 1;
  // start serving.
  while (ServerRunning) {
// clear the set of file descriptors
    FD_ZERO(&SelectSock);
    FD_SET(MasterSocket_fd, &SelectSock); // add MasterSocket_fd file descriptor to the set of file descriptors
    max_fd = MasterSocket_fd; // in case that MasterSocket is the only file descripter in the set assign it to the highest position in set
    // Initialize timeval structure
    SelectTimeOut.tv_sec = TimeOut;
    SelectTimeOut.tv_usec = 0;
    //assign connection the status of select()
    connection = select(max_fd + 1, &SelectSock, 0, 0, &SelectTimeOut);
    // if there are no other file descriptors in the set, no activity from the client
    if ( connection  == 0) {
      fprintf(stdout, "%ld secs passed, no echo request. Blocking at select().\n", TimeOut);
    } else if (connection > 0) { // incoming message
      if (FD_ISSET(MasterSocket_fd, &SelectSock)) {
        clientlen = sizeof(ClientAddress);
        // accept client sockaddr_in structure and connect node
        Socket_fd = accept(MasterSocket_fd, (struct sockaddr *) &ClientAddress, &clientlen);
        // error checking
        if (Socket_fd < 0) {
          perror("Error when calling accept()");
        } else {
          // add the new socket to the set
          FD_SET(Socket_fd, &SelectSock);
          // check if the new socket has a greater position than the highest index mx_fd
          max_fd = (max_fd < Socket_fd) ? Socket_fd : max_fd;
        }
        printf("New connection, socket fd is %d, IP is : %s, PORT : %d \n" , Socket_fd , inet_ntoa(ClientAddress.sin_addr) , ntohs(ClientAddress.sin_port));
        FD_CLR(MasterSocket_fd, &SelectSock);
      }
      // go through the set of readable socket file descriptors and read data from socket
      for (i = 0; i < max_fd + 1; i++) {
        //check if socket descripter is in the set
        if (FD_ISSET(i, &SelectSock)) {
          bzero(buffer, BuffSize);
          // read the client message
          message = read(i, buffer, BuffSize);
          if (message < 0) {
            perror("Error when reading from socket");
          }
          // echo the message back to the client
          message = write(Socket_fd, buffer, strlen(buffer));
          if (message < 0) {
            perror("Error when writing to socket");
          }
          //clean up
          close(i);
          FD_CLR(i, &SelectSock);
        }
      }
    }
  }

  printf("Shuting down server.\n");
  // clean up
  close(MasterSocket_fd);
  exit(-1);
}


