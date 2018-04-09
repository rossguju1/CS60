/* Ross Guju
*  CS60 Networks
*  Lab1 - Socket Programming
*  TCPServer_v4.c
*/


#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <sys/time.h> 

#define BuffSize 2000



int main(int argc, char **argv) {
  int MasterSocket_fd; 
  int Socket_fd, max_fd; 
  unsigned short PortNumber; 
  unsigned int clientlen; 
  struct sockaddr_in ServerAddress; 
  struct sockaddr_in ClientAddress; 
  char buffer[BuffSize]; 
  int opt_val; 
  int message; 
  int connection; \
  int ServerRunning;
  fd_set SelectSock;
  long TimeOut;
  struct timeval SelectTimeOut;
  int i;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <TimeOut (Seconds)> <Port>\n", argv[0]);
    exit(-1);
  }
  PortNumber = atoi(argv[2]);
  TimeOut = atoi(argv[1]);

  MasterSocket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (MasterSocket_fd < 0) {
    perror("opening socket failed");
    exit(-1);
  }


opt_val = 1;
setsockopt(MasterSocket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
  
  
  bzero((char *) &ServerAddress, sizeof(ServerAddress));

  ServerAddress.sin_family = AF_INET;

  ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  ServerAddress.sin_port = htons(PortNumber);

 
  if (bind(MasterSocket_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0) {
    perror("Error when Binding");
    exit(-1);
  }

 
 if (listen(MasterSocket_fd, 3) < 0) {
  perror("Error when listening");
  exit(-1);
} 
printf("Listening on Port %d\n", PortNumber);
printf("Waiting for connections......\n");

  ServerRunning = 1;
  
  while (ServerRunning) {

  FD_ZERO(&SelectSock);          
  FD_SET(MasterSocket_fd, &SelectSock);
  max_fd = MasterSocket_fd;
    SelectTimeOut.tv_sec = TimeOut;
    SelectTimeOut.tv_usec = 0;
  
 connection = select(max_fd + 1, &SelectSock, 0, 0, &SelectTimeOut);
         
    if ( connection  == 0) {
      fprintf(stdout, "%ld secs passed, no echo request. Blocking at select().\n", TimeOut);
    } else if (connection > 0) {
      if (FD_ISSET(MasterSocket_fd, &SelectSock)) {
        clientlen = sizeof(ClientAddress);

      Socket_fd = accept(MasterSocket_fd, (struct sockaddr *) &ClientAddress, &clientlen);
      if (Socket_fd < 0) {
       perror("Error when calling accept()");
      } else {
        FD_SET(Socket_fd, &SelectSock);
        max_fd = (max_fd < Socket_fd)? Socket_fd : max_fd;
      }
      printf("New connection, socket fd is %d, IP is : %s, PORT : %d \n" , Socket_fd , inet_ntoa(ClientAddress.sin_addr) , ntohs(ClientAddress.sin_port));  
      FD_CLR(MasterSocket_fd, &SelectSock);
    }
      for (i = 0; i < max_fd + 1; i++) {
        if(FD_ISSET(i, &SelectSock)) {
          bzero(buffer, BuffSize);
          message = read(i, buffer, BuffSize);
          if (message < 0) {
              perror("Error when reading from socket");
            }
            message = write(Socket_fd, buffer, strlen(buffer));
              if (message < 0) {
                    perror("Error when writing to socket");
               }
              close(i);
              FD_CLR(i, &SelectSock);
        }  
      }
    }
  }

  printf("Terminating server.\n");
  close(MasterSocket_fd);
  exit(-1);
}

