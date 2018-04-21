/*
 * client/mrt_client.h: implementation of MRT client socket 
 * interfaces. 
 *
 * CS60, March 2018. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include "mrt_client.h"

/**************** global variables ****************/
client_tcb_t *p_client_tcb_table[MAX_TRANSPORT_CONNECTIONS];
int overlay_connection;

/**************** functions ****************/

// TODO - mrt client initialization
void mrt_client_init(int conn) {
}

// TODO - Create a client sock
int mrt_client_sock(unsigned int client_port) {
	return 1;
}

// TODO - Connect to the server
int mrt_client_connect(int sockfd, unsigned int server_port) {
	return -1;
}

// TODO - Send data to a mrt server
int mrt_client_send(int sockfd, void *data, unsigned int length) {
	return -1;
}

// TODO - Disconnect from the server
int mrt_client_disconnect(int sockfd) {
	return 1;
}

// TODO - Close client
int mrt_client_close(int sockfd) {
	return 1;
}


// TODO - Thread handles incoming segment
/**
* @example
*
* while(1)
1) Get segment using function mnp_recvseg(), kill thread if something
 went wrong
2) Find tcb to handle this segment by looking up tcbs using the dest
 port in the segment header
3) switch(tcb state)
  case CLOSED: break;  
  case SYNSENT:
    if(seg.header.type==SYNACK && tcb->svrport==seg.header.srcport)
      set tcb state to CONNECTED
    break;
  case CONNECTED: 
    if segBuf.type == DATAACK && tcb.svrport==segBuf.srcport
      if segBuf.ack_num >= tcb.sendBufHead.seq_num
        update send buf with ack_num
        send new segments in send buffer
    break;
  case FINWAIT:
    if(seg.header.type==FINACK && tcb->svrport==seg.header.srcport)
      set tcb state to CLOSED
      break;
*
*/
void *seghandler(void *arg) {

}
