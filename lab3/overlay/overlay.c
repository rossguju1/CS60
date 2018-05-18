/*	Ross Guju
 * overlay/overlay.c: implementation of an ON process. It has
 * following steps:
 * 1. connect to all neighbors
 * 2. start listen_to_neighbor threads, each of which keeps
 *    receiving packets from a neighbor and forwarding the received
 * 		packets to the MNP process.
 * 3. waits for the connection from MNP process.
 * 4. after connecting to a MNP process, keep receiving
 *    sendpkt_arg_t structures from the MNP process and sending the
 *    received packets out to the overlay network.
 *
 *  ROSS GUJU
 *
 * CS60, March 2018.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>
#include <assert.h>

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"
#include "overlay.h"
#include "neighbortable.h"

/**************** local constants ****************/
// start the ON processes on all the overlay hosts within this period of time
#define OVERLAY_START_DELAY 60

/**************** global variables ****************/
nbr_entry_t* nt; // neighbor table
int network_conn; // TCP connection

/**************** local function prototypes ****************/
void* waitNbrs(void* arg);
int connectNbrs();
void* listen_to_neighbor(void* arg);
void waitNetwork();
void overlay_stop();

/**************** main function ****************/
// entry point of the overlay
int main() {
	/* start overlay initialization */
	printf("Overlay: Node %d initializing...\n \n", topology_getMyNodeID());

	/* create a neighbor table */
	nt = nt_create();
	/* initialize network_conn to -1, means no MNP process is connected yet */
	network_conn = -1;

	/* register a signal handler which is sued to terminate the process */
	signal(SIGINT, overlay_stop);

	/* print out all the neighbors */
	int nbrNum = topology_getNbrNum();
	for (int i = 0; i < nbrNum; i++) {
		printf("Overlay: neighbor %d:%d\n\n", i + 1, nt[i].nodeID);
	}

	/* start the waitNbrs thread to wait for incoming connections from neighbors with larger node IDs */
	pthread_t waitNbrs_thread;
	pthread_create(&waitNbrs_thread, NULL, waitNbrs, (void *) 0);

	/* wait for other nodes to start */
	sleep(OVERLAY_START_DELAY);

	/* connect to neighbors with smaller node IDs */
	connectNbrs();

	/* wait for waitNbrs thread to return */
	pthread_join(waitNbrs_thread, NULL);

 	/* create threads listening to all the neighbors */
	for (int i = 0; i < nbrNum; i++) {
		int *idx = (int *) malloc(sizeof(int));
		*idx = i;
		pthread_t nbr_listen_thread;
		pthread_create(&nbr_listen_thread, NULL, listen_to_neighbor, (void *) idx);
	}
	printf("Overlay: node initialized...\n\n");
	printf("Overlay: waiting for connection from NP process...\n\n");

	/* waiting for connection from MNP process */
	waitNetwork();
}

/**************** functions ****************/
// thread function waiting for incoming connections from
// neighbors with larger node IDs
 

 // waitNbrs()
void* waitNbrs(void* arg) {
int nbrNum, myNodeID;

// define socket file descriptors and node ID

int socket_fd, nbr_socket_fd, nbr_socket_ID;

// used for socket()
struct sockaddr_in socket_address;

//get number of neighbors
nbrNum = topology_getNbrNum();

// get the local host node ID
myNodeID = topology_getMyNodeID();

// counter used for listen to incoming connections
int counter;
counter = 0;

// get socket descriptor which will be the connection number
socket_fd = socket(AF_INET, SOCK_STREAM, 0);

int* neighbor_Array;
neighbor_Array = topology_getNbrArray();
int l;
l = 0;
// index by l and get number of incoming connections
while(l < nbrNum){
if (myNodeID < neighbor_Array[l]){
	counter++;
		}
		l++;
	}

	printf("Number of incoming neighbors is %d\n", counter );

// error checking
if (socket_fd < 0) {
	printf("%s\n\n", "socket_fd failed ");
	pthread_exit(NULL);
	}




  // get address information for the server
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = INADDR_ANY;
  socket_address.sin_port = htons(CONNECTION_PORT);

// bind to socket and check if successful
 if (bind(socket_fd, (struct  sockaddr*) &socket_address, sizeof(struct sockaddr_in)) == -1){
 	printf("%s\n\n", "bind failed");
 		}

 printf("overlay.c LISTENING\n \n");


 while(counter > 0){

// listen up to max number of nodes
 if (listen(socket_fd, MAX_NODE_NUM) < 0) {
 	printf("could not listen\n \n");
 	}
// initialize new socket address
struct sockaddr_in new_sock;
int llen = sizeof(new_sock);

socklen_t *llen2 = (socklen_t*)(&llen);

nbr_socket_fd = accept(socket_fd, (struct sockaddr*) &new_sock, llen2);

		

if (nbr_socket_fd < 0) {
		printf("%s \n\n", "nbr accept failed");
		pthread_exit(NULL);
}
		 
// get node ID for the new socket
nbr_socket_ID = topology_getNodeIDfromip(&(new_sock.sin_addr));
		printf("recieved connection from %d\n", nbr_socket_ID);
		

			if ( nt_addconn(nt, nbr_socket_ID, nbr_socket_fd) < 0) {
				printf("%s\n\n", "file descriptor was not added");
			}
			
			// descrament counter which is the number of connecting neighbors
		counter--;
	}
	//exit
	close(socket_fd);
	pthread_exit(NULL);

return NULL;

}
// connectNbrs() 
// input : none
// output : successful 1 or fail -1
// 
// connect to neighbors with smaller node IDs

int connectNbrs() {
// define variables used
int nbrNum, myNodeID;

int socket_fd1;
socket_fd1 = 0;

// get number of neighbors
nbrNum = topology_getNbrNum();

// get my node
myNodeID = topology_getMyNodeID();

printf("connectNbrs() ~>nbrNum~> %d connectNbrs() ~>myNodeID~> %d \n",nbrNum , myNodeID);

int i;

for (i = 0; i< nbrNum; i++) {
	// only consider node ID's that are smaller than local ID
	if (myNodeID > nt[i].nodeID) {
		// make socket
	socket_fd1 = socket(AF_INET, SOCK_STREAM, 0);
	// check if socket was made
	if (socket_fd1 < 0){
		printf("%s\n", "socket was not made");
		return -1;
	}

	struct sockaddr_in server_adress;
	// clear server_address variable
	memset(&server_adress, 0, sizeof(server_adress));
	memcpy(&server_adress.sin_addr, &(nt[i].nodeIP), sizeof(nt[i].nodeIP));

	// get information
	server_adress.sin_family = AF_INET;
	server_adress.sin_port = htons(CONNECTION_PORT);


	printf("send connection request to %s\n", inet_ntoa(server_adress.sin_addr));

	// connect to socket
	if (connect(socket_fd1, (struct sockaddr*)&server_adress, sizeof(server_adress)) < 0) {
		printf("failed to connect~> socket ~> %d \n\n", socket_fd1);
		return -1;
		close(socket_fd1);
	}	
	// get the new socket ID for the node
	int new_ID;
	new_ID = topology_getNodeIDfromip(&(server_adress.sin_addr));
	// add the ID and connection number to the neighbor table
	if (nt_addconn(nt, new_ID, socket_fd1) < 0) {
		printf("%s\n\n", "file descriptor was not added");
		return -1;
	}
	printf("overlay.c ~> connectNbrs()~>added socket_fd to TCP Connection~> %d \n\n", socket_fd1);

		}	
	}
return 1;
}


// listen_to_neighbor()
// intput : arg
// return thread function for listening to all the neighbors

void* listen_to_neighbor(void* arg) {

 int* idx;
 idx = (int *)arg;
// define the packet to be recieved then forward
struct packet pkt;

nbr_entry_t nbr = nt[*idx];

while(recvpkt(&pkt, nbr.conn)) {
   forwardpktToMNP(&pkt, network_conn);
}	
// clean up and exit
close(nbr.conn);
	pthread_exit(NULL);
}

// waitNetwork()
// input : none
// reuturn:  wait for connection from MNP process

void waitNetwork() {

struct packet pkt;

printf("%s\n", "At waitNetwork");

// int* nodeNbrIDArray;
// nodeNbrIDArray = topology_getNbrArray();

int nbrNum;

// inialize and get length size
struct sockaddr_in socket_address;
socklen_t len;
len = sizeof(struct sockaddr_in);

// TODO set up socket


// define next Node
int nextNode; //= (int*)malloc(sizeof(int));

int socket_fd;
// get  the socket descriptor
socket_fd = socket(AF_INET, SOCK_STREAM, 0);

// clear memmery for socket_address
memset(&socket_address, 0, sizeof(socket_address));


	// get socket information
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = INADDR_ANY;
  socket_address.sin_port = htons(OVERLAY_PORT);

// bind 
if (bind(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) < 0){
	printf("%s\n", "didnt bind");
}


while(1) {

// listen 
 if (listen(socket_fd, MAX_NODE_NUM) < 0) {
 	printf("socket_fd :%d failed when listening \n\n", socket_fd);
 }


// accept incomingrequests
network_conn = accept(socket_fd, (struct sockaddr*) &socket_address, &len);
if (network_conn < 0) {
	printf("net_conn :~> %d failed when accpeting\n\n", network_conn);
	close(network_conn);
}

// send packet
while (getpktToSend(&pkt, &nextNode, network_conn) > 0) {

// get neighbor numbers
nbrNum = topology_getNbrNum();

int i;
// send packet to all NODES
if( nextNode == BROADCAST_NODEID ){

	// if node connection is non zero
	for (i = 0; i < nbrNum; i++) {
		
		if (nt[i].conn >= 0){
		printf("%s\n", "COND_1 ~> if nextNode == BROADCAST_NODEID");
		printf("going through nbr table to send pkt: nt[i].conn (sock fd) ~> %d \n", nt[i].conn);
		sendpkt(&(pkt), nt[i].conn);
		printf("%s\n", "pkt sent!\n");
		}
	  }
	} else {

	for (i = 0; i < nbrNum; i++) {
		if(nt[i].nodeID == nextNode && nt[i].conn >= 0){
              sendpkt(&(pkt), nt[i].conn);
  		printf("going through nbr table to send pkt: nt[i].conn (sock fd) ~> %d \n", nt[i].nodeID);
		printf("%s\n", "COND_2	if nt[i].nodeID == *nextNode && nt[i].conn >= 0");
		printf("%s\n", "pkt sent!\n");

          }

      	}
	  }
	}
	// close netword
close(network_conn);
}
//close listening socket
close(socket_fd);

return;

}



// overlay_stop()
// input : none
// output: none
// clean up malloc'd information
void overlay_stop() {

int nbrNum;
// get neighbor number
nbrNum = topology_getNbrNum();
int i;
// if the connection is none negative close
for(i = 0; i < nbrNum; i++) {
		close(nt[i].conn);
}
if(network_conn!= -1) {
	close(network_conn);
}
// clean up
nt_destroy(nt);

exit(0);
}


