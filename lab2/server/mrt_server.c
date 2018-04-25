/*
 * server/mrt_server.c: implementation of MRT server socket
 * interfaces.
 * Ross Guju
 * Lab2
 *
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
#include <wchar.h>
#include <string.h>
#include <memory.h>
#include "mrt_server.h"

/**************** global variables ****************/
server_tcb_t *p_server_tcb_table[MAX_TRANSPORT_CONNECTIONS]; // TCB table
int overlay_connection; // overlay_connection
pthread_mutex_t mutex;
pthread_cond_t cond;
void *closewait_handle( void *args );
void close_wait_timeout(server_tcb_t *server_TCB);
void *timer_handler(void *args);
server_tcb_t *fetch_TCB(unsigned int svr_portNum);

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
//
/******* mrt_server_sock()******/

int mrt_server_sock(unsigned int port) {


  server_tcb_t *server_TCB;

  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

    if (p_server_tcb_table[i] == NULL ) {

// initialize variables
      server_TCB = p_server_tcb_table[i] = malloc(sizeof(server_tcb_t));

      server_TCB->state = SVR_CLOSED;
      server_TCB->svr_portNum = port;
      server_TCB->recvBuf = malloc(RECEIVE_BUF_SIZE);
      server_TCB->bufMutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
      server_TCB->usedBufLen = 0;
      pthread_mutex_init(server_TCB->bufMutex, NULL);

      return i;

    }

  }

  return -1;
}

/******* mrt_server_accept *****/

int mrt_server_accept(int sockfd) {
//  struct timeval timer;
  server_tcb_t *server_TCB;
  server_TCB = p_server_tcb_table[sockfd];
//timer.tv_sec = 0;
  //timer.tv_usec = ACCEPT_POLLING_INTERVAL / 1000;

  if (server_TCB->state == SVR_CLOSED) {
    server_TCB->state = SVR_LISTENING;



    printf("%s\n", "Waiting to accept\n" );
    //sleep(1);
    for (;;) {
// if in connected state return 1
      if (server_TCB->state == SVR_CONNECTED) {
        printf("accepted state & connnected\n");
        return 1;
      }
    }
  }
  sleep(1);
  return -1;
}


int mrt_server_recv(int sockfd, void *buf, unsigned int length) {

  int i, l;

  if (sockfd < 0 || sockfd > MAX_TRANSPORT_CONNECTIONS) {
    printf("something went wrong.");
    return -1;
  }

  server_tcb_t *server_TCB;

  server_TCB = p_server_tcb_table[sockfd];
// initialize input buffer
  char* data;
  data = (char*) buf;

  if (length > server_TCB->usedBufLen) {
    sleep(RECVBUF_POLLING_INTERVAL);
  }



  pthread_mutex_lock(server_TCB->bufMutex);

// get the buffer.

  for (i = 0; i < length; i++) {
    data[i] = server_TCB->recvBuf[i];
  }

  for (i = 0; i < server_TCB->usedBufLen; i++) {
    server_TCB->recvBuf[l] = server_TCB->recvBuf[i];
    l++;
  }

  server_TCB->usedBufLen -= length;
  return 1;
  pthread_mutex_unlock(server_TCB->bufMutex);

}

/*****mrt_server_close*****/

int mrt_server_close(int sockfd) {
  if (sockfd < 0 || sockfd > MAX_TRANSPORT_CONNECTIONS) {
    printf("something went wrong.");
    return -1;
  }
  server_tcb_t *server_TCB = p_server_tcb_table[sockfd];

  if (server_TCB != NULL) {
    printf("mrt_server_close(int sockfd)\n");
    switch (server_TCB->state) {
    case (SVR_LISTENING):
//close in the listening state
      printf("current state is not SVR_CLOSED\n");
      printf("Changing to SRV_CLOSED\n");
      break;

    case (SVR_CONNECTED):
//close in the sever connect state
      printf("sockfd is in connecged state\n");
      printf("Changing to SRV_CLOSED\n");
      break;

    default:
      printf("Changing current state to SRV_CLOSED\n");
      break;
    }
  }
// clean up
  server_TCB->state = SVR_CLOSED;
  pthread_mutex_destroy(server_TCB->bufMutex);
  free(server_TCB->recvBuf);
  free(server_TCB->bufMutex);
  free(server_TCB);
  server_TCB->recvBuf = NULL;
  server_TCB->bufMutex = NULL;
  server_TCB = NULL;


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

/****seghandler()*****/

void *seghandler(void *arg) {

  seg_t segment;      //REceived segment
  seg_t segment_send;      //Sendingsegment


  server_tcb_t *server_TCB;
  while (1) {
    server_TCB = fetch_TCB(segment.header.dest_port);
    mnp_recvseg(overlay_connection, &segment);

    if (server_TCB == NULL) {
      continue;
    }

    printf("recv string: %s from overlay \n", segment.data);

    switch (server_TCB->state) {

    case SVR_CLOSED:
      break;
      //case when state is listening
    case SVR_LISTENING:

      if (segment.header.type == SYN) {
        memset(&segment_send, 0, sizeof(seg_t));

        server_TCB->expect_seqNum = segment.header.seq_num;
        segment_send.header.length = 0;
        server_TCB->client_portNum = segment.header.src_port;
        server_TCB->state = SVR_CONNECTED;
        segment_send.header.dest_port = server_TCB->client_portNum;
        segment_send.header.src_port = server_TCB->svr_portNum;
        segment_send.header.type = SYNACK;

        mnp_sendseg(overlay_connection, &segment_send);

      }
      break;
      //case when stat is connected
    case SVR_CONNECTED:
      if (segment.header.type == SYN && server_TCB->client_portNum == segment.header.src_port) {
        memset(&segment_send, 0, sizeof(seg_t));

        segment_send.header.type = SYNACK;
        segment_send.header.seq_num = server_TCB->expect_seqNum;
        segment_send.header.length = 0;
        segment.header.src_port = server_TCB->svr_portNum;
        segment.header.dest_port = server_TCB->client_portNum;


        mnp_sendseg(overlay_connection, &segment_send);
      }
      // if server recieves data
      else if (segment.header.type == DATA && server_TCB->client_portNum == segment.header.src_port) {

        // pthread_mutex_lock(server_TCB->bufMutex);



        if (segment.header.seq_num == server_TCB->expect_seqNum) {
          if ((segment.header.length + server_TCB->usedBufLen) < RECEIVE_BUF_SIZE) {

            // for (int k = 0; k < segment.header.length; k++) {
            //   printf("%c", segment.data[k]  );
            // }

           

            memcpy(&server_TCB->recvBuf[server_TCB->usedBufLen], segment.data, segment.header.length);
            server_TCB->usedBufLen += segment.header.length;
            server_TCB->expect_seqNum = segment.header.seq_num + segment.header.length;
          }
        } else {


          memset(&segment_send, 0, sizeof(seg_t));
          segment_send.header.type = DATAACK;
          segment_send.header.src_port = server_TCB->svr_portNum;
          segment_send.header.dest_port = server_TCB->client_portNum;
          segment_send.header.ack_num = server_TCB->expect_seqNum;
          //pthread_mutex_lock(server_TCB->bufMutex);
          //memmove((server_TCB->recvBuf + server_TCB->usedBufLen), segment.data, segment.header.length);

          segment_send.header.length = 0;
          //server_TCB->usedBufLen += segment.header.length;
          // server_TCB->expect_seqNum += segment.header.length;



          mnp_sendseg(overlay_connection, &segment_send);

        }
      }
      else if (segment.header.type == FIN && server_TCB->client_portNum == segment.header.src_port) {
        segment_send.header.type = FINACK;
        server_TCB->state = SVR_CLOSEWAIT;
        //server_TCB->state = SVR_CLOSEWAIT;
        segment_send.header.ack_num = server_TCB->expect_seqNum;
        segment_send.header.length = 0;
        segment_send.header.type = FINACK;

        segment_send.header.src_port = server_TCB->svr_portNum;
        segment_send.header.dest_port = server_TCB->client_portNum;
        // mnp_recvseg(overlay_connection, &segment);
        mnp_sendseg(overlay_connection, &segment_send);
        close_wait_timeout(server_TCB);

      }
      break;

    case SVR_CLOSEWAIT:
      if (segment.header.type == FIN && server_TCB->client_portNum == segment.header.src_port) {
        segment_send.header.type = FINACK;
        segment_send.header.ack_num = server_TCB->expect_seqNum;
        segment_send.header.length = 0;
        segment_send.header.src_port = server_TCB->svr_portNum;
        segment_send.header.dest_port = server_TCB->client_portNum;
        mnp_sendseg(overlay_connection, &segment_send);
      }
      break;

    default:
      break;
    }
  }
  return NULL;
}

/**** fetch_TCB()*****/


server_tcb_t *fetch_TCB(unsigned int svr_portNum)
{

  server_tcb_t *server_TCB;
// get the associated sever data structure
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
    server_TCB = p_server_tcb_table[i];
    if (server_TCB != NULL && server_TCB->svr_portNum == svr_portNum) {

      return server_TCB;
    }
    else if (server_TCB == NULL) {
      continue;
    }
  }
  return NULL;

}
void *timer_handler(void *args)
{
  // handles cloasing time
  sleep(CLOSEWAIT_TIME);
  server_tcb_t *server_TCB;
  server_TCB = (server_tcb_t *)args;

  pthread_mutex_lock(server_TCB->bufMutex);
  server_TCB->state = SVR_CLOSED;
  server_TCB->usedBufLen = 0;
  pthread_mutex_unlock(server_TCB->bufMutex);

  return NULL;
}

void close_wait_timeout(server_tcb_t *server_TCB)
{
  pthread_t pthreadID;

//time out
  if (pthread_create(&pthreadID, NULL, timer_handler, (void *)server_TCB) != 0) {

    printf("close_wait_timeout create pthread failed\n");
    exit(0);
  }
}


