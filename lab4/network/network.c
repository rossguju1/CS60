/*
 * network/network.c: implementation of the MNP process.
 *
 * 	Ross Guju
 *
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
#include "../common/seg.h"
#include "../topology/topology.h"
#include "network.h"
#include "routing_table.h"
#include "nbrcosttable.h"
#include "dvtable.h"
//#include "routing_table.h"

/**************** constants ****************/
#define NETWORK_WAITTIME 60

/**************** global variables ****************/
int overlay_connection; 		//connection to the overlay
int transport_connection; 	//connection to the transport
nbr_cost_entry_t *nbr_cost_table; //neighbor cost table
dv_t *dv_table; 									//distance vector table
pthread_mutex_t *dv_mutex; 				//dvtable mutex
routingtable_t *routing_table; 		//routing table
pthread_mutex_t *routingtable_mutex; //routing_table mutex

/**************** local function prototypes ****************/
int connectToOverlay();
void *routeupdate_daemon(void *arg);
void *pkthandler(void *arg);
void waitTransport();
void network_stop();

/**************** main function ****************/
/* TODO: entry point for the network:
 *  1) initialize neighbor cost table, distance vector table, mutex
 *     routing table, connections to overlay and transport
 *  2) print out the three tables
 *  3) set up signal handler for SIGINT
 * 	4) set up overlay connection
 *  5) create packet handling thread
 *  6) create route update thread
 *  7) wait NETWORK_WAITTIME for routes to be established and then
 *     print routing table
 *  8) wait for the MRT process to connect (waitTransport())
 */
int main(int argc, char *argv[]) {
	printf("network layer is starting, pls wait...\n");
	/*  1) initialize neighbor cost table, distance vector table, mutex
	      routing table, connections to overlay and transport
	      */


	nbr_cost_table = nbrcosttable_create();

	nbrcosttable_print(nbr_cost_table);
	printf("%s\n", "made nbr table");

	dv_table = dvtable_create();
	printf("%s\n", "made dvtable table");

	dv_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	printf("%s\n", "dv mutex malloc'd");

	pthread_mutex_init(dv_mutex, NULL);

	printf("%s\n", "dv_mutex init");


	routing_table = routingtable_create();
	printf("%s\n", "routing_table mutex made");

	routingtable_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

	printf("%s\n", "routingtable_mutex malloc'd");

	pthread_mutex_init(routingtable_mutex, NULL);

	printf("%s\n", "routingtable_mutex init");


	transport_connection = -1;

	overlay_connection = -1;

	/* 2) print out the three tables*/

	printf("%s\n", "main 1");

	nbrcosttable_print(nbr_cost_table);

	printf("%s\n", "main 2");

	dvtable_print(dv_table);

	printf("%s\n", "main 3");

	routingtable_print(routing_table);

	printf("%s\n", "main 4");



	/* 3) set up signal handler for SIGINT */

	printf("%s\n", "main 5");

	signal(SIGINT, network_stop);

	printf("%s\n", "main 6");

	/* 4) set up overlay connection */

	overlay_connection = connectToOverlay();

	if (connectToOverlay() < 0) {

		printf("%s\n", "Could not connect to overlay");

		exit(-1);
	}

	pthread_t pkt_handler_pthread;

	pthread_create(&pkt_handler_pthread, NULL, pkthandler, (void*)0);

	pthread_t routingUpdatePthread;

	pthread_create(&routingUpdatePthread, NULL, routeupdate_daemon, (void*)0);

	printf("%s\n", "MNP initiated");

	printf("%s\n", "waiting NETWORK_WAITTIME");

	sleep(NETWORK_WAITTIME);

	/* 7) wait NETWORK_WAITTIME for routes to be established and then print routing table*/
	printf("%s\n", "about to print out routing_table");

	routingtable_print(routing_table);

	/* 8) wait for the MRT process to connect (waitTransport())*/

	// wait connection from MRT process
	printf("waiting for connection from MRT process...\n");

	waitTransport();

	return 0;
}

/**************** local functions ****************/

// This function is used to for the network layer process to
// connect to the local overlay process on port OVERLAY_PORT.
// return connection descriptor if success, -1 otherwise.
// Pseudocode
// 1) Fill in sockaddr_in for socket
// 2) Create socket
// 3) return the socket descriptor
int connectToOverlay() {

	// fill in sockaddr_in for socket

	struct sockaddr_in servaddr;

	servaddr.sin_family = AF_INET;

	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	servaddr.sin_port = htons(OVERLAY_PORT);

	int overlay_conn = socket(AF_INET, SOCK_STREAM, 0);

	// error checking

	if (overlay_conn < 0) {

		printf("%s\n", "couldnt connect to overlay");
		return -1;
	}

	// try to connect to overlay

	if (connect(overlay_conn, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
		printf("%s\n", "connection to overlay failed");
		return -1;
	}

	// successfully connected

	return overlay_conn;
}

// TODO: This thread handles incoming packets from the ON process.
// Pseudocode
// 1) while recv packet from overlay connection
// 2) 	if it is a ROUTE_UPDATE packet
// 3) 		fetch data (distance vector) and src node ID
// 4) 		lock dv table and update dv table
// 5) 		update routing table if necessary with lock
// 6)     release locks
// 7)   if it is a MNP packet to itself
// 8)     forward it to MRT layer w/ forwardsegToMRT()
// 9) 	if it is a MNP packet to others
// 10)    find the node ID of the next hop based on the routing table
// 11)    send it to the next hop w/ overlay_sendpkt()
// 2) close overlay conn
// 3) exit thread

void *pkthandler(void *arg) {

	// defines variables

	mnp_pkt_t packet;

	pkt_routeupdate_t pktUpdate;

	seg_t segment;

	int i;

	int dvCost;

	int nbrCost;

	int nextRoutingNode;



	int MyNodeID = topology_getMyNodeID();

	printf("%s\n", "about to start pkthandler");

	// while recv packet from overlay connection

	while (overlay_recvpkt(&packet, overlay_connection) > 0) {

		if (packet.header.type == ROUTE_UPDATE) {

			// mutex_lock threads

			pthread_mutex_lock(dv_mutex);

			pthread_mutex_lock(routingtable_mutex);

			// clear pktUpdate and save data from packet.data

			memset(&pktUpdate, 0, packet.header.length);

			memcpy(&pktUpdate, packet.data, packet.header.length);

			printf("%s\n", packet.data);

			printf("%s\n", "packet.header.type == ROUTE_UPDATE");

			// go through entries in pktUpdate and update

			for (i = 0; i < pktUpdate.entryNum; i++) {


				routeupdate_entry_t* routing_update;

				routing_update = &pktUpdate.entry[i];

				//printf("packet.header.src_nodeID => %d routing_update->nodeID => %d \n", packet.header.src_nodeID, routing_update->nodeID);

				// set dvtable cost from src_nodeID to routing_update_>nodeID

				dvtable_setcost(dv_table, packet.header.src_nodeID, routing_update->nodeID, routing_update->cost);

				// routeupdate algorithm where we compare dvCost and nbrCost

				dvCost = dvtable_getcost(dv_table, MyNodeID, routing_update->nodeID);


				nbrCost = (nbrcosttable_getcost(nbr_cost_table, packet.header.src_nodeID) + routing_update->cost);

				//printf("nbrCost => %d and dvCost => %d\n", nbrCost, dvCost);

				// if dvCost is bigger than nbrCost then update the next hop

				if (dvCost > nbrCost) {


					dvtable_setcost(dv_table, MyNodeID, routing_update->nodeID, nbrCost);


					routingtable_setnextnode(routing_table, routing_update->nodeID, packet.header.src_nodeID);
				}
			}

			// unlock mutex

			pthread_mutex_unlock(dv_mutex);

			pthread_mutex_unlock(routingtable_mutex);

		} else if (packet.header.dest_nodeID == MyNodeID && packet.header.type == MNP) {

			// clear segment and save packet.data

			memset(&segment, 0, packet.header.length);

			memcpy(&segment, packet.data, packet.header.length);

			// forward segment to MRT

			forwardsegToMRT(transport_connection, packet.header.src_nodeID, &segment);

			printf("%s\n", "packet.header.dest_nodeID == MyNodeID && packet.header.type == MNP");

			// if the pckate header is not my nodeID and header.type is MNP send to next hop

		} else if (packet.header.dest_nodeID != MyNodeID && packet.header.type == MNP) {

			// mutex lock

			pthread_mutex_lock(routingtable_mutex);

			//  send to next hop

			nextRoutingNode = routingtable_getnextnode(routing_table, packet.header.dest_nodeID);

			// unlock mutex

			pthread_mutex_unlock(routingtable_mutex);

			// send move packet to next hop overlay

			overlay_sendpkt(nextRoutingNode, &packet, overlay_connection);

			printf("%s\n", "packet.header.dest_nodeID != MyNodeID && packet.header.type == MNP");
		}
	}

	// close overlay_connection and exit pthread

	close(overlay_connection);

	printf("%s\n", "pkthandler close");

	pthread_exit(NULL);
}

// TODO: This thread sends out route update packets every
// ROUTEUPDATE_INTERVAL. The route update packet contains this
// node's distance vector.
// Broadcasting is done by set the dest_nodeID in packet header as
// BROADCAST_NODEID and use overlay_sendpkt() to send it out.
// Pseudocode
// 1) get my node ID and number of neighbors
// 2) while(1)
//    Fill in mnp_pkt header with myNodeID, BROADCAST_NODEID and
//			ROUTE_UPDATE
//    Cast the MNP packet data as pkt_routeupdate_t type, set the
//      entryNum as the number of neighbors
// 		Lock the dv table, put distance vector into the packet data
//    Unlock the dv table
//    Set the length in packet header: sizeof(entryNum) + entryNum*
//      sizeof(routeupdate_entry_t)
//    if(overlay_sendpkt(BROADCAST_NODEID,&ru,overlay_conn < 0)
//      close(overlay_conn)
//      exit
//    Sleep ROUTEUPDATE_INTERVAL
void *routeupdate_daemon(void *arg) {

	// printf("%s\n", "@ routeupdate_Daemon1");

	// initialize variables

	mnp_pkt_t packet;

	pkt_routeupdate_t routingUpdate;

	packet.header.dest_nodeID = BROADCAST_NODEID;

	packet.header.length = sizeof(pkt_routeupdate_t);

	packet.header.type = ROUTE_UPDATE;

	routingUpdate.entryNum = topology_getNodeNum();

	int* nodeARRAY = topology_getNodeArray();

	packet.header.src_nodeID = topology_getMyNodeID();

	memset(routingUpdate.entry, 0, MAX_NODE_NUM * sizeof(routeupdate_entry_t));

	int i;

	// printf("%s\n", " @ routeupdate_Daemon2");

	while (1) {

		// pthread lock

		pthread_mutex_lock(dv_mutex);

		// go through routingUpdate entries get distance variable cost

		for (i = 0; i < routingUpdate.entryNum; i++) {

			// printf("%s\n", " @ routeupdate_Daemon4");

			routeupdate_entry_t *entry;

			entry = &routingUpdate.entry[i];

			entry->nodeID = nodeARRAY[i];

			printf("entry->nodeID => %d\n", entry->nodeID);

			// printf("%s\n", " @ routeupdate_Daemon5");

			// if (packet.header.src_nodeID == NULL) {
			// 	printf("%s\n", "packet.header.src_nodeID is null");

			// } else if (entry->nodeID == NULL) {

			// 	printf("%s\n", "entry->nodeID");
			// }

			printf("packet.header.src_nodeID => %d\n", packet.header.src_nodeID);

			if (dv_table == NULL) {

				printf("dv_table is null\n");
			}

			entry->cost = dvtable_getcost(dv_table, packet.header.src_nodeID, entry->nodeID);

			// printf("%s\n", " @ routeupdate_Daemon6");

		}

		// printf("%s\n", " @ routeupdate_Daemon7");

		// clear packet and copy data

		memset(packet.data, 0, sizeof(pkt_routeupdate_t));

		memcpy(packet.data, &routingUpdate, sizeof(pkt_routeupdate_t));

		pthread_mutex_unlock(dv_mutex);

		// printf("%s\n", " @ routeupdate_Daemon8");

		// sned snedpkt to overlay

		int status = overlay_sendpkt(BROADCAST_NODEID, &packet, overlay_connection);

		// printf("%s\n", " @ routeupdate_Daemon9");
		//
		// check status for error checking

		if (status < 0) {

			break;
		}

		// printf("%s\n", " @ routeupdate_Daemon10");
		//
		// sleep fpr ROUTEUPDATE_INTERVAL

		sleep(ROUTEUPDATE_INTERVAL);

		printf("%s\n", " @ routeupdate_Daemon11");
	}

	// printf("%s\n", " @ routeupdate_Daemon12");

	// close overlat and free nodeARRAY

	close(overlay_connection);

	free(nodeARRAY);

	// overlay_connection = -1;

	pthread_exit(NULL);
}

// TODO: this function opens a port on NETWORK_PORT and waits for
// the TCP connection from local MRT process.
// Pseudocode
// 1) create a socket listening on NETWORK_PORT
// 2) while (1)
// 3)   accept an connection from local MRT process
// 4) 	while (getsegToSend()) keep receiving segment and
//      destination node ID from MRT
// 5) 		encapsulate the segment into a MNP packet
// 6)     find the node ID of the next hop based on the routing table
// 7)     send the packet to next hop using overlay_sendpkt()
// 8)   close the connection to local MRT
void waitTransport() {

	// printf("%s\n", "@ waitTransport 1");

	// initialize sockaddr_in

	struct sockaddr_in socket_address;

	int sock_fd;

	int nextHop;

	// error checking for socket

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

		printf("socket failed to be made during waitTransport\n\n");

		exit(-1);
	}

	// printf("%s\n", "@ waitTransport 2");

	// clear socket_address by setting memmory to 0

	memset(&socket_address, 0, sizeof(struct sockaddr_in));

	// fill in socket_address information

	socket_address.sin_port = htons(NETWORK_PORT);

	socket_address.sin_family = AF_INET;

	socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//error checking

	int bind_status;

	// printf("%s\n", "@ waitTransport 3");

	bind_status = bind(sock_fd, (struct sockaddr*) &socket_address, sizeof(struct sockaddr_in));

	// check blind status for error checking

	if (bind_status == -1) {

		printf("%s\n", "bind failed in waitTransport");
	}

	// printf("%s\n", "@ waitTransport 4");

	int listen_status;

	// listen to accepting sockets

	listen_status = listen(sock_fd, MAX_NODE_NUM);

	// printf("%s\n", "@ waitTransport 5");

	// error checking for listen_Status

	if (listen_status < 0) {

		printf("%s\n", "listen failed in waitTransport");

		close(sock_fd);
	}

	// printf("%s\n", "@ waitTransport 6");

	// accept transport_ connection

	while (1) {

		mnp_pkt_t packet;

		transport_connection = accept(sock_fd, NULL, NULL);

		// printf("%s\n", "@ waitTransport 7");

		// error checking

		if (transport_connection < 0) {

			printf("%s\n", "transport_connection failed to accept socket in waitTransport");

			break;
		}

		// get segment to send to overlay process

		// fill in packet header information

		packet.header.type = MNP;

		packet.header.src_nodeID = topology_getMyNodeID();

		packet.header.length = sizeof(seg_t);

		int segment_status;

		// printf("%s\n", "@ waitTransport 8");

		segment_status = getsegToSend(transport_connection, &packet.header.dest_nodeID, (seg_t*) &packet.data);

		// printf("%s\n", "@ waitTransport 9");

		// while there is a segment to send, lock mutex, get next hop node and then send to overlay

		while (segment_status > 0) {

			pthread_mutex_lock(routingtable_mutex);

			// printf("%s\n", "@ waitTransport 10");

			nextHop = routingtable_getnextnode(routing_table, packet.header.dest_nodeID);

			// printf("%s\n", "@ waitTransport 11");
			pthread_mutex_unlock(routingtable_mutex);

			// printf("%s\n", "@ waitTransport 12");
			overlay_sendpkt(nextHop, &packet, overlay_connection);
			// printf("%s\n", "@ waitTransport 13");
		}

		// close

		close(transport_connection);

		transport_connection = -1;
		// printf("%s\n", "@ waitTransport 14");
	}
	printf("%s\n", "@ waitTransport 15");
	close(sock_fd);
}


// TODO: This function stops the MNP process. It closes all the
// connections and frees all the dynamically allocated memory.
// It is called when the MNP process receives a signal SIGINT.
// 1) close overlay connection if it exists
// 2) close the connection to MRT if it exists
// 3) destroy tables, free mutex
// 2) exit
void network_stop() {

	// clean up

	// printf("%s\n", "network_stop 2");

	routingtable_destroy(routing_table);

	pthread_mutex_destroy(routingtable_mutex);

	close(overlay_connection);

	close(transport_connection);

	// printf("%s\n", "network_stop 3");

	free(routingtable_mutex);

	dvtable_destroy(dv_table);

	// printf("%s\n", "network_stop 4");


	pthread_mutex_destroy(dv_mutex);

	free(dv_mutex);

	nbrcosttable_destroy(nbr_cost_table);

	// printf("%s\n", "network_stop 5");


	exit(-1);

}

