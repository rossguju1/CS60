/*
 * network/network.c: implementation of the MNP process.
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
#include "nbrcosttable.h"
#include "dvtable.h"
#include "routing_table.h"

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

	nbr_cost_entry_t *nbr_cost_table = nbrcosttable_create();


	dv_t* dvtable = dvtable_create();


	dv_mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init(routingtable_mutex, NULL);


	routingtable_t* routing_table = routingtable_create();

	transport_connection = -1;

	overlay_connection = -1;

	/* 2) print out the three tables*/

	nbrcosttable_print(nbr_cost_table);

	dvtable_print(dvtable);

	routingtable_print(routing_table);

	/* 3) set up signal handler for SIGINT */

	signal(SIGINT, network_stop);

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

	struct sockaddr_in servaddr;

	servaddr.sin_family = AF_INET;

	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	servaddr.sin_port = htons(OVERLAY_PORT);

	int overlay_conn = socket(AF_INET, SOCK_STREAM, 0);

	if (overlay_conn < 0) {

		return -1;
	}

	if (connect(overlay_conn, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {

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

	mnp_pkt_t packet;

	pkt_routeupdate_t pktUpdate;

	seg_t segment;

	int i;

	int dvCost;

	int nbrCost;

	int nextRoutingNode;

	int MyNodeID = topology_getMyNodeID();

	while (overlay_recvpkt(&packet, overlay_connection) > 0) {

		if (packet.header.type == ROUTE_UPDATE) {

			pthread_mutex_lock(dv_mutex);

			pthread_mutex_lock(routingtable_mutex);

			memset(&pktUpdate, 0, packet.header.length);

			memcpy(&pktUpdate, packet.data, packet.header.length);

			for (i = 0; i < pktUpdate.entryNum; i++) {

				routeupdate_entry_t* routing_update;

				routing_update = &pktUpdate.entry[i];

				dvtable_setcost(dv_table, packet.header.src_nodeID, routing_update->nodeID, routing_update->cost);

				dvCost = dvtable_getcost(dv_table, MyNodeID, routing_update->nodeID);

				nbrCost = (nbrcosttable_getcost(nbr_cost_table, packet.header.src_nodeID) + routing_update->cost);

				if (dvCost > nbrCost) {

					dvtable_setcost(dv_table, MyNodeID, routing_update->nodeID, nbrCost);

					routingtable_setnextnode(routing_table, routing_update->nodeID, packet.header.src_nodeID);
				}
			}
			pthread_mutex_unlock(dv_mutex);

			pthread_mutex_unlock(routingtable_mutex);

		} else if (packet.header.dest_nodeID == MyNodeID && packet.header.type == MNP) {

			memset(&segment, 0, packet.header.length);

			memcpy(&segment, packet.data, packet.header.length);

			forwardsegToMRT(transport_connection, packet.header.src_nodeID, &segment);

		} else if (packet.header.dest_nodeID != MyNodeID && packet.header.type == MNP) {

			pthread_mutex_lock(routingtable_mutex);

			nextRoutingNode = routingtable_getnextnode(routing_table, packet.header.dest_nodeID);

			pthread_mutex_unlock(routingtable_mutex);

			overlay_sendpkt(nextRoutingNode, &packet, overlay_connection);
		}
	}
	close(overlay_connection);

	overlay_connection = -1;

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

	while (1) {

		pthread_mutex_lock(dv_mutex);

		for (i = 0; i < routingUpdate.entryNum; i++) {

			routeupdate_entry_t *entry;

			entry = &routingUpdate.entry[i];

			entry->nodeID = nodeARRAY[i];

			entry->cost = dvtable_getcost(dv_table, packet.header.src_nodeID, entry->nodeID);

		}
		memset(packet.data, 0, sizeof(pkt_routeupdate_t));

		memcpy(packet.data, &routingUpdate, sizeof(pkt_routeupdate_t));

		pthread_mutex_unlock(dv_mutex);

		int status = overlay_sendpkt(BROADCAST_NODEID, &packet, overlay_connection);

		if (status < 0) {

			break;
		}

		sleep(ROUTEUPDATE_INTERVAL);
	}

	free(nodeARRAY);

	close(overlay_connection);

	overlay_connection = -1;

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

	struct sockaddr_in socket_address;

	int sock_fd;

	int nextHop;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

		printf("socket failed to be made during waitTransport\n\n");

		exit(-1);
	}

	memset(&socket_address, 0, sizeof(struct sockaddr_in));

	socket_address.sin_port = htons(NETWORK_PORT);

	socket_address.sin_family = AF_INET;

	socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

	int bind_status;

	bind_status = bind(sock_fd, (struct sockaddr*) &socket_address, sizeof(struct sockaddr_in));

	if (bind_status == -1) {

		printf("%s\n", "bind failed in waitTransport");
	}

	int listen_status;

	listen_status = listen(sock_fd, MAX_NODE_NUM);

	if (listen_status < 0) {

		printf("%s\n", "listen failed in waitTransport");

		close(sock_fd);
	}

	while (1) {

		mnp_pkt_t packet;

		transport_connection = accept(sock_fd, NULL, NULL);

		if (transport_connection < 0) {

			printf("%s\n", "transport_connection failed to accept socket in waitTransport");

			break;
		}

		packet.header.type = MNP;

		packet.header.src_nodeID = topology_getMyNodeID();

		packet.header.length = sizeof(seg_t);

		int segment_status;

		segment_status = getsegToSend(transport_connection, &packet.header.dest_nodeID, (seg_t*) &packet.data);

		while (segment_status > 0) {

			pthread_mutex_lock(routingtable_mutex);

			nextHop = routingtable_getnextnode(routing_table, packet.header.dest_nodeID);

			pthread_mutex_unlock(routingtable_mutex);

			overlay_sendpkt(nextHop, &packet, overlay_connection);

		}

		close(transport_connection);

		transport_connection = -1;

	}

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

	close(overlay_connection);

	close(transport_connection);

	routingtable_destroy(routing_table);

	pthread_mutex_destroy(routingtable_mutex);

	free(routingtable_mutex);

	dvtable_destroy(dv_table);

	pthread_mutex_destroy(dv_mutex);

	free(dv_mutex);

	nbrcosttable_destroy(nbr_cost_table);

	exit(-1);


}

