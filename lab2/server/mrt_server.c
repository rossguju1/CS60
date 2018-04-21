/*
 * server/mrt_server.c: implementation of MRT server socket 
 * interfaces. 
 *
 * CS60, March 2018. 
 */

#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <memory.h>
#include "mrt_server.h"

/**************** global variables ****************/
server_tcb_t *p_server_tcb_table[MAX_TRANSPORT_CONNECTIONS]; // TCB table
int overlay_connection; // overlay_connection


/**************** functions ****************/

// TODO - mrt server initialization
void mrt_server_init(int conn) {
	/* initialize the global variables */
	for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
		p_server_tcb_table[i] = NULL;
	}
	overlay_connection = conn;

	/* Create a thread for handling connections */
	pthread_t seghandler_thread;
	pthread_create(&seghandler_thread, NULL, seghandler, (void *) 0);
}

// TODO - Create a server sock and setting the recv buffer
int mrt_server_sock(unsigned int port) {
	return 0;
}

// TODO - Accept overlay_connection from mrt client
int mrt_server_accept(int sockfd) {
	return -1;
}

// TODO - Receive data from a mrt client
int mrt_server_recv(int sockfd, void *buf, unsigned int length) {
	return 1;
}

// TODO - Close the mrt server and Free all the space
int mrt_server_close(int sockfd) {
	return 1;
}


// TODO - Thread handles incoming segments
/**
 * @example
 *
 * while(1)
  Get segment using mnp_recvseg()
  Get tcb by looking up tcb table by dest port in segment header
  switch(tcb state)
    case CLOSED: break;

    case LISTENING:
      if(seg.header.type==SYN)
        tcb.client_port = seg.header.srcport
        send SYNACK to client (syn_received call)
        tcb.state = CONNECTED
      break;

    case CONNECTED:
      if(seg.header.type==SYN && tcb.client_port==seg.header.srcport)
        duplicate SYN received
        send SYNACK to client (syn_received call)

      else if(seg.header.type==DATA && tcb.client_port == seg.header.srcport)
        update tcb with data received (data_recieved call)

      else if(seg.header.type==FIN && tcb.client_port==seg.header.srcport)
        send FINACK with fin_received call
        tcb state = CLOSEWAIT
        spawn thread to handle CLOSEWAIT state transition (see closewait below)
      break;

    case CLOSEWAIT: 
      if (seg.header.type==FIN && tcb.client_port == seg.header.srcport)
        send FINACK with fin_received call
      break;
 *
 */
void *seghandler(void *arg) {
  
}
