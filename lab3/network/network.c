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
/* TODO: entry point for the network: 
 *  1) set up signal handler for SIGINT
 * 	2) set up overlay connection
 *  3) create packet handling thread
 *  4) create route update thread
 */
int main(int argc, char *argv[]) {

	return 0;
}

/**************** local functions ****************/

// TODO: this function is used to for the network layer process to 
// connect to the local overlay process on port OVERLAY_PORT. 
// return connection descriptor if success, -1 otherwise. 
// Pseudocode
// 1) Fill in sockaddr_in for socket
// 2) Create socket
// 3) return the socket descriptor 
int connectToOverlay() {

	return 0;
}

// TODO: This thread handles incoming packets from the ON process.
// We will implement the actual packet handling in lab 4. 
// In this lab, it only receives packets.  
// Pseudocode
// 1) while recv packet from overlay connection
//      print where it came from
// 2) close overlay conn
// 3) kill thread
void *pkthandler(void *arg) {

}

// TODO: This thread sends out route update packets every 
// ROUTEUPDATE_INTERVAL. In this lab, we do not add any data in the 
// packet. In lab 4, we will add distance vector as the packet data. 
// Broadcasting is done by set the dest_nodeID in packet header as 
// BROADCAST_NODEID and use overlay_sendpkt() to send the packet out 
// using BROADCAST_NODEID.  
// Pseudocode
// 1) while(1)
//    Fill in mnp_pkt header with myNodeID, BROADCAST_NODEID and 
//			ROUTE_UPDATE
//    Set mnp_pkt.header.length = 0
//    if(overlay_sendpkt(BROADCAST_NODEID,&ru,overlay_conn < 0)
//      close(overlay_conn)
//      exit
//    Sleep ROUTEUPDATE_INTERVAL
void *routeupdate_daemon(void *arg) {

}

// TODO: This function stops the MNP process. It closes all the 
// connections and frees all the dynamically allocated memory. 
// It is called when the MNP process receives a signal SIGINT.
// 1) close overlay connection if it exists
// 2) exit
void network_stop() {

}

