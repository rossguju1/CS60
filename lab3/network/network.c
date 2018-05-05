/*
 * network/network.c: implementation of the MNP process. 
 * ROSS GUJU
 * CS60, March 2018. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <assert.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <unistd.h>

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"
#include "network.h"

/**************** global variables ****************/
int overlay_conn; //connection to the overlay

/**************** local function prototypes ****************/
int connectToOverlay(); 
void *routeupdate_daemon(void *arg); 
void *pkthandler(void *arg); 
void network_stop(); 

/**************** main function ****************/
/*  entry point for the network: 

 */
int main(int argc, char *argv[]) {

	pthread_t pkthandler_pthread;
	pthread_t routerhandler_pthread;


printf("\t \t NETWORK.c main() is runnning \n \n");

overlay_conn = -1;



signal(SIGINT, network_stop);


overlay_conn = connectToOverlay();

printf("network.c ~> connectToOverlay() ~> output socket_fd = %d \n \n \n", overlay_conn);

pthread_create(&pkthandler_pthread, NULL, pkthandler, (void*) 0);

pthread_create(&routerhandler_pthread, NULL, routeupdate_daemon, (void*) 0);


printf("\t %s \n\n\n", "pkthandler_pthread & routerhandler_pthread created");
while(1 && overlay_conn > 0) {
	sleep(60);
	
}
return 0;
}

/**************** local functions ****************/

// TODO: this function is used to for the network layer process to 
// connect to the local overlay process on port OVERLAY_PORT. 
// return connection descriptor if success, -1 otherwise. 
// input : NONE
// output : success socket descriptor or fail -1

int connectToOverlay() {

	//initilze variables
	int socket_fd;
	char hostname[1024];

	struct sockaddr_in socket_address;
	struct hostent *hostptr;
	
	// get socket descriptor
socket_fd = socket(AF_INET, SOCK_STREAM, 0);

// get hostname in netork format
gethostname(hostname, 1024);

printf("hostname: used for network.c ~>%s\n\n", hostname);

hostptr = gethostbyname(hostname);



// copy info 
  memcpy(&socket_address.sin_addr.s_addr, hostptr->h_addr_list[0], sizeof(struct in_addr)); 
socket_address.sin_family = AF_INET;
socket_address.sin_port = htons(OVERLAY_PORT);

// connect to overlat
	if (connect(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == 0) {
	printf("\t network~> socket_fd is connected ~> socket_fd = %d \n\n\n", socket_fd);
		return socket_fd;
	}

		
return -1;
}

// TODO: This thread handles incoming packets from the ON process.
// We will implement the actual packet handling in lab 4. 
// In this lab, it only receives packets.  

void *pkthandler(void *arg) {

mnp_pkt_t pkt;

while(overlay_recvpkt(&pkt, overlay_conn) > 0) {

printf("~~pkthandler() ~> packet was recieved and came from neighbor %d \n\n\n", pkt.header.src_nodeID);
}

close(overlay_conn);

overlay_conn = -1;

pthread_exit(NULL);

return NULL;
}

//This thread sends out route update packets every 
// ROUTEUPDATE_INTERVAL. In this lab, we do not add any data in the 
// packet. In lab 4, we will add distance vector as the packet data. 
// Broadcasting is done by set the dest_nodeID in packet header as 
// BROADCAST_NODEID and use overlay_sendpkt() to send the packet out 
// using BROADCAST_NODEID.  
void *routeupdate_daemon(void *arg) {


mnp_pkt_t pkt;
pkt_routeupdate_t route_update;


while(1) {

	// initialize packet info
	pkt.header.src_nodeID = topology_getMyNodeID();
	pkt.header.dest_nodeID = BROADCAST_NODEID;
	pkt.header.type = ROUTE_UPDATE;
	pkt.header.length = sizeof(pkt_routeupdate_t);
	route_update.entryNum = 0;

	// clear memory then copy info
	memset(route_update.entry, 0, MAX_NODE_NUM * sizeof(routeupdate_entry_t));
	memcpy(pkt.data, &route_update, sizeof(pkt_routeupdate_t));

	// send pack to overlay
	if (overlay_sendpkt(BROADCAST_NODEID, &pkt, overlay_conn) < 0) {
		printf("%s\n \n", "closing routeupdate_daemon");
		close(overlay_conn);
		
	} else {
		printf("PACKET SENT!! to Overlay\n \n ");
	}

	sleep(ROUTEUPDATE_INTERVAL);

}
return NULL;

}

// network_stop()
// input : NONE
//return : This function stops the MNP process. It closes all the 
// connections and frees all the dynamically allocated memory. 

void network_stop() {

	close(overlay_conn);
	exit(0);

}

