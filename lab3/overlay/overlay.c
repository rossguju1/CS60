/*
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
	printf("Overlay: Node %d initializing...\n", topology_getMyNodeID());

	/* create a neighbor table */
	nt = nt_create();
	/* initialize network_conn to -1, means no MNP process is connected yet */
	network_conn = -1;

	/* register a signal handler which is sued to terminate the process */
	signal(SIGINT, overlay_stop);

	/* print out all the neighbors */
	int nbrNum = topology_getNbrNum();
	for (int i = 0; i < nbrNum; i++) {
		printf("Overlay: neighbor %d:%d\n", i + 1, nt[i].nodeID);
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

	/* at this point, all connections to the neighbors are created */

	/* create threads listening to all the neighbors */
	for (int i = 0; i < nbrNum; i++) {
		int *idx = (int *) malloc(sizeof(int));
		*idx = i;
		pthread_t nbr_listen_thread;
		pthread_create(&nbr_listen_thread, NULL, listen_to_neighbor, (void *) idx);
	}
	printf("Overlay: node initialized...\n");
	printf("Overlay: waiting for connection from NP process...\n");

	/* waiting for connection from MNP process */
	waitNetwork();
}

/**************** functions ****************/
// TODO: thread function waiting for incoming connections from 
// neighbors with larger node IDs
// 1) Get number of neighbors, nbrNum
// 2) Get my node ID, myNodeID
// 3) Count number of incoming neighbors, incoming_neighbors
// 4) Create socket to listen on at port CONNECTION_PORT
// 5) while(incoming_neighbors > 0)
//      accept connection on socket
//      get id of neighbor, nbrID
//      add nbrID and connection to neighbor table
// 6) close the listening socket
void* waitNbrs(void* arg) {

	pthread_exit(NULL);
}

// TODO: connect to neighbors with smaller node IDs
// 1) nbrNum = get neighbor num
// 2) myNodeID = getMyNodeID
// 3) for each neighbor
//      if myNodeID > nt[i].nodeID
//        make a socket on CONNECTION_PORT and connect to it
//        add this neighbor to neighbor table with connection
// 4) return 1
int connectNbrs() {

	return 1;
}

// TODO: thread function for listening to all the neighbors
// 1) while(receive packet on neighbor connection)
//      foward packet to MNP
// 2) close neighbor conn
// 3) kill thread
void* listen_to_neighbor(void* arg) {

	pthread_exit(NULL);
}

// TODO: wait for connection from MNP process 
// 1) Create socket on OVERLAY_PORT to listen to
// 2) while (1)
//      net_conn = accept connection on socket
//      while(get packets to send on net_conn)
//        nbrNum = get neighbor num
//        if(nextNode == BROADCAST_NODEID)
//          for each nbr
//            if nt[i].conn >= 0
//              sendpkt(pkt,nt[i].conn)
//        else
//          for each nbr
//            if(nt[i].nodeID == nextNodeID && nt[i].conn >= 0)
//              sendpkt(pkt,nt[i].conn)
//      close(net_conn)
void waitNetwork() {

}

// TODO: 
// 1) if(net_conn != -1)
//      close(net_conn)
// 2) destory neighbor table
// 3) exit(0)
void overlay_stop() {

}


