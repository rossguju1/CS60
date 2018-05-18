/*
  CS60SU18

  lab4

  mrt_client.c

  ROSS GUJU
 */


#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include "../topology/topology.h"
#include "../common/seg.h"
#include "mrt_client.h"

int create_TCB_table(unsigned int PORT);



/**************** global variables ****************/
client_tcb_t *p_client_tcb_table[MAX_TRANSPORT_CONNECTIONS];

int network_connection;

pthread_mutex_t mutex;

pthread_cond_t cond;

/************* functions ********************/

void sendBuffer_timeout(client_tcb_t *table);

void *sendingBuffer_timer(void *table);

void sendBufferRecvAck(client_tcb_t *table, unsigned int ack_seqnum);


void sendBuffer_append_segment(client_tcb_t *table, segBuf_t *segmentBuffer);

void sendBuffer_sending(client_tcb_t *table);

client_tcb_t *fetch_client_TCB(unsigned int ClientPort);

void sendBuffer_destroy(client_tcb_t *table);





/**
 * This function initializes the TCB table marking all entries NULL.
 * It also initializes a global variable for the overlay TCP socket
 * descriptor "conn" used as an input parameter for mnp_sendseg()
 * and mnp_recvseg(). Finally, the function starts the seghandler
 * thread to handle incoming segments.
 *
 * Note: There is only one seghandler at the client side which
 * handles call connections for the client.
 *
 * @param conn - socket descriptor
 */



// // client transport control block (TCB), containing information
// // of one overlay connection at the client side.
// typedef struct client_tcb {
//  unsigned int svr_nodeID;        //node ID of server, obtained by  topology_getNodeIDfromname()
//  unsigned int svr_portNum;       //port number of server
//  unsigned int client_nodeID;     //node ID of client, obtained by  topology_getNodeIDfromname()
//  unsigned int client_portNum;    //port number of client
//  unsigned int state;             //state of client

//  // fields below are used for data transmission
//  unsigned int next_seqNum;
//  pthread_mutex_t* bufMutex;
//  segBuf_t* sendBufHead;
//  segBuf_t* sendBufunSent;
//  segBuf_t* sendBufTail;
//  unsigned int unAck_segNum;
// } client_tcb_t;

void mrt_client_init(int conn)
{
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

    p_client_tcb_table[i] = NULL;
  }

  network_connection = conn;

  pthread_t seghandler_pthread;

  pthread_create(&seghandler_pthread, NULL, seghandler, (void*) 0);

}


/**
 * This function looks up the client TCB table to find the first
 * NULL entry, and creates a new TCB entry using malloc() for that
 * entry. All fields in the TCB are initialized, e.g., TCB state is
 * set to CLOSED and the server port set to the function call
 * parameter server port. The TCB table entry index should be
 * returned as the new socket ID to the server and be used to
 * identify the connection on the server side. If no entry in the
 * TCB table is available the function returns -1.
 *
 * @param port - port number
 * @return 1 for success, -1 for failure
 */

int mrt_client_sock(unsigned int client_port)
{

  int socket_fd;

  socket_fd = create_TCB_table(client_port);

  if (socket_fd < 0) {

    printf("%s\n", "client Transprt Control Block (TCB) is full");

    return -1;
  }

  // initialize TCB_table

  client_tcb_t* TCB_table = p_client_tcb_table[socket_fd];

  TCB_table->state = CLIENT_CLOSED;

  TCB_table->client_nodeID = topology_getMyNodeID();

  TCB_table->unAck_segNum = 0;

  TCB_table->next_seqNum = 0;

  TCB_table->sendBufunSent = 0;

  TCB_table->sendBufTail = 0;

  TCB_table->sendBufHead = 0;

  TCB_table->svr_nodeID = -1;

  pthread_mutex_t *sendBuffer_mutex = (pthread_mutex_t* ) malloc(sizeof(pthread_mutex_t));

  pthread_mutex_init(sendBuffer_mutex, NULL);

  TCB_table->bufMutex = sendBuffer_mutex;

  return socket_fd;
}


/**
 * This function is used to connect to the server. It takes the
 * socket ID, server node ID, the server's port number as input
 * parameters. The socket ID is used to find the TCB entry.
 * This function sets up the TCB's server port number and a SYN
 * segment to send to the server using mnp_sendseg(). After the SYN
 * segment is sent, a timer is started. If no SYNACK is received
 * after SYNSEG_TIMEOUT timeout, then the SYN is retransmitted. If
 * SYNACK is received, return 1. Otherwise, if the number of SYNs
 * sent > SYN_MAX_RETRY, transition to CLOSED state and return -1.
 *
 * @param sockfd - socket file descriptor
 * @param server_port - server port for connection
 * @return 1 for successful change; -1 for failure
 */

int mrt_client_connect(int sockfd, int nodeID, unsigned int server_port)
{

  client_tcb_t *TCB_table = p_client_tcb_table[sockfd];

  if (TCB_table == NULL) {

    printf("%s\n", "mrt_client_connect failed to find p_client_tcb_table[socket_fd]");

    return -1;
  }

  seg_t client_syn;

  int syn_retry;

  // use FSM to control flow

  switch (TCB_table->state) {

  case CLIENT_CLOSED:
    // if in Client closed, set the TCB_table server port and ID

    TCB_table->svr_nodeID = nodeID;

    TCB_table->svr_portNum = server_port;

    memset(&client_syn, 0, sizeof(client_syn));

    // initialize syn message header to send to server

    client_syn.header.length = 0;

    client_syn.header.seq_num = 0;

    client_syn.header.type = SYN;

    client_syn.header.dest_port = TCB_table->svr_portNum;

    client_syn.header.src_port = TCB_table->client_portNum;

    // send segment to MNP process

    int send_seg_status = mnp_sendseg(network_connection, TCB_table->svr_nodeID, &client_syn);

    // error checking

    if (send_seg_status == -1) {

      printf("%s\n", "mrt_client failed to send connection segment to MNP");

    } else {

      printf("%s\n\n", "Client sent SYN");
    }

    TCB_table->state = CLIENT_SYNSENT;

    syn_retry = 0;

    while (syn_retry != SYN_MAX_RETRY) {

      // use select function to poll the SYN_timeout timter

      select(0, 0, 0, 0, &(struct timeval) {.tv_usec = SYN_TIMEOUT});

      // retry sending the SYN segment

      if (TCB_table->state != CLIENT_CONNECTED) {

        printf("trying to connect to server\n\n");

        mnp_sendseg(network_connection, TCB_table->svr_nodeID, &client_syn),

                    syn_retry++;

      } else if (TCB_table->state == CLIENT_CONNECTED) {

        printf("connected to server\n\n");

        return 1;

      } else if (syn_retry == SYN_MAX_RETRY) {

        TCB_table->state = CLIENT_CLOSED;

        return -1;
      }

      break;

    case CLIENT_CONNECTED:

      return -1;

    case CLIENT_SYNSENT:

      return -1;

    case CLIENT_FINWAIT:

      return -1;

    default:

      return -1;
    }
  }
  return -1;
}




/**
 * This function sends data to a MRT server.
 *
 * @param sockfd
 * @param data
 * @param length
 * @return 1 for success; -1 for failure
 */

int mrt_client_send(int sockfd, void* data, unsigned int length)
{
  int segment_number;

  int i;

  char *send_data;

  client_tcb_t *TCB_table = p_client_tcb_table[sockfd];

  if (TCB_table == NULL) {

    printf("%s\n", "mrt_client_connect failed to find p_client_tcb_table[socket_fd]");

    return -1;
  }

  switch (TCB_table->state) {

  case CLIENT_CONNECTED:

    //int segment_number;
    //int i;

    segment_number = (length / MAX_SEG_LEN) + 1;

    printf("segment_number before length mod MAX_seg_LEN = %d\n", segment_number);

    // if (length % MAX_SEG_LEN) {

    //   segment_number++;
    //   printf("segment_number++ = %d\n\n", segment_number);
    // }


    // go through teh parsezd segment chunks

    for (i = 0; i < segment_number; i++) {

      // malloc memmory for segment buffer

      segBuf_t *segmentBuffer = (segBuf_t*)malloc(sizeof(segBuf_t));

      memset(segmentBuffer, 0, sizeof(segBuf_t));

      // update header source and destination port

      segmentBuffer->seg.header.src_port = TCB_table->client_portNum;

      segmentBuffer->seg.header.dest_port = TCB_table->svr_portNum;

      if ( i == (segment_number - 1)) {

        segmentBuffer->seg.header.length = length % MAX_SEG_LEN;

      } else {

        segmentBuffer->seg.header.length = MAX_SEG_LEN;

        segmentBuffer->seg.header.type = DATA;

        send_data = (char*) data;

        //     for (int j=0; j<headerLength; j++){
        //   newSegBuf->seg.data[j] = datachar[ i*MAX_SEG_LEN + j];
        // }

        memcpy(segmentBuffer->seg.data, &send_data[i * MAX_SEG_LEN], segmentBuffer->seg.header.length);

        sendBuffer_append_segment(TCB_table, segmentBuffer);
      }


    }

    sendBuffer_sending(TCB_table);

    return 1;

  case CLIENT_CLOSED:

    return -1;

  case CLIENT_SYNSENT:

    return -1;

  case CLIENT_FINWAIT:

    return -1;

  default:

    return -1;
  }

  return -1;
}



/**
 * This function disconnects from the server. It takes the socket
 * ID as an input parameter. The socket ID is used to find the TCB
 * entry in the TCB table. This function sends a FIN segment to the
 * server. After the FIN segment is sent the state should transition
 * to FINWAIT and a timer started. If the state == CLOSED after the
 * timeout the FINACK was successfully received. Else, if after a
 * number of retries FIN_MAX_RETRY the state is still FINWAIT then
 * the state transitions to CLOSED and -1 is returned.
 *
 * @param sockfd
 * @return 1 for success; -1 for failure
 */

int mrt_client_disconnect(int sockfd)
{

  seg_t finished;

  client_tcb_t *TCB_table = p_client_tcb_table[sockfd];

  // using FSM to control flow

  switch (TCB_table->state) {

  case CLIENT_CONNECTED:

    // update header for the final segnment to send to mrt_server

    memset(&finished, 0, sizeof(seg_t));

    finished.header.length = 0;

    finished.header.dest_port = TCB_table->svr_portNum;

    finished.header.src_port = TCB_table->client_portNum;

    finished.header.type = FIN;

    mnp_sendseg(network_connection, TCB_table->svr_nodeID, &finished);

    printf("%s\n", "sent finished ");

    TCB_table->state = CLIENT_FINWAIT;

    printf("%s\n", "switching to FINWATE state");

    for (int i = 0; i < FIN_MAX_RETRY; i++) {

      // use select function to wait FIN_TIMEOUT tune

      select(0, 0, 0, 0, &(struct  timeval) {.tv_usec = FIN_TIMEOUT});

      if (TCB_table->state == CLIENT_CLOSED) {

        TCB_table->next_seqNum = 0;

        TCB_table->svr_nodeID = -1;

        TCB_table->svr_portNum = 0;

        sendBuffer_destroy(TCB_table);

        return 1;
      } else {

        printf("%s\n\n", "client POLLING AND SENT AGAIN");

        mnp_sendseg(network_connection, TCB_table->svr_nodeID, &finished);
      }
    }

    TCB_table->state = CLIENT_CLOSED;

    return -1;

  case CLIENT_CLOSED:

    return -1;

  case CLIENT_SYNSENT:

    return -1;

  case CLIENT_FINWAIT:

    return -1;

  default:

    return -1;

  }

  return -1;
}



/**
 * This function calls free() to free the TCB entry. It marks that
 * entry in TCB as NULL and returns 1 if succeeded (i.e., was in the
 * right state to complete a close) and -1 if fails (i.e., in the
 * wrong state).
 *
 * @param sockfd
 * @return 1 for success; -1 for failure
 */

int mrt_client_close(int sockfd)
{
  client_tcb_t *TCB_table = p_client_tcb_table[sockfd];

  switch (TCB_table->state) {

  case CLIENT_CLOSED:

    free(TCB_table->bufMutex);

    free(p_client_tcb_table[sockfd]);

    p_client_tcb_table[sockfd] = NULL;

    return 1;

  case CLIENT_CONNECTED:

    return -1;

  case CLIENT_SYNSENT:

    return -1;

  case CLIENT_FINWAIT:

    return -1;

  default:

    return -1;

  }
}


/**
 * This is a thread started by mrt_client_init(). It handles all the
 * incoming segments from the server. The design of seghanlder is an
 * infinite loop that calls mnp_recvseg(). If mnp_recvseg() fails
 * then the overlay connection is closed and the thread is
 * terminated. Depending on the state of the connection when a
 * segment is received (based on the incoming segment) various
 * actions are taken. See the client FSM for more details.
 *
 * @param arg
 * @return 1 for success; -1 for failure
 */

void *seghandler(void* arg)
{

  seg_t segmentBuffer;

  int source_nodeID;

  while (1)
  {

    // while we are recieiving segments

    if (mnp_recvseg(network_connection, &source_nodeID, &segmentBuffer) < 0) {

      close(network_connection);

      pthread_exit(NULL);
    }

    // fetch the associated Tcb_table

    client_tcb_t *TCB_table = fetch_client_TCB(segmentBuffer.header.dest_port);

    if (TCB_table == NULL) {

      printf("%s\n", "mrt_client seghandler couldnt fetch the TCB => no port according to table");

      continue;
    }

    // use FSM to control flow

    switch (TCB_table->state) {

    case CLIENT_CONNECTED:

      printf("%s\n\n", "CLIENT IN CONNECTED");

      if (TCB_table->svr_nodeID == source_nodeID && TCB_table->svr_portNum == segmentBuffer.header.src_port && segmentBuffer.header.type == DATAACK) {

        if (TCB_table->sendBufHead != NULL && segmentBuffer.header.ack_num >= TCB_table->sendBufHead->seg.header.seq_num) {

          printf("%s\n\n", "sending Buffer RECEIVED ACKNOWLEDGEMENT");
          sendBufferRecvAck(TCB_table, segmentBuffer.header.ack_num);

          printf("%s\n\n", "sending buffer after sending recieved acknowledgement");
          sendBuffer_sending(TCB_table);
        }

      } else {

        printf("segmentBuffer.header.ack_num => %d AND TCB_table->sendBufHead->seg.header.seq_num => %d \n", segmentBuffer.header.ack_num, TCB_table->sendBufHead->seg.header.seq_num);

        printf("%s\n\n", "mrt_client seghandler -> no DATA recieved");
      }

      break;

    case CLIENT_SYNSENT:

      printf("%s\n\n", "CLIENT IN SYNSENT");

      if (segmentBuffer.header.type == SYNACK && TCB_table->svr_portNum == segmentBuffer.header.src_port && TCB_table->svr_nodeID == source_nodeID) {

        TCB_table->state = CLIENT_CONNECTED;
        printf("%s\n\n", "SWITCHING CLIENT TO CONNECTED STATE");
      }

      break;

    case CLIENT_FINWAIT:

      printf("%s\n\n", "CLIENT IN FINWAIT");

      if (segmentBuffer.header.type == FINACK && TCB_table->svr_portNum == source_nodeID && TCB_table->svr_portNum == segmentBuffer.header.src_port ) {

        printf("%s\n\n", "SWITCHING TO CLIENT CLOSED");

        TCB_table->state = CLIENT_CLOSED;

      } else {

        printf("%s\n\n", "IN CLIENT FINWAIT AND NO FINACK SENT");
      }

      break;

    case CLIENT_CLOSED:

      printf("%s\n\n", "CLIENT IN CLOSED");

      break;

    default:

      break;

    }
  }
}


/************** create_TCB_table() ******************/


int create_TCB_table(unsigned int PORT)
{
  int i;

  for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

    // malloc entriesr and initialize to NULL

    if (p_client_tcb_table[i] == NULL) {

      p_client_tcb_table[i] = (client_tcb_t*) malloc(sizeof(client_tcb_t));

      p_client_tcb_table[i]->client_portNum = PORT;

      // return index

      return i;

    } else if (p_client_tcb_table[i] != NULL && p_client_tcb_table[i]->client_portNum == PORT) {

      continue;
    }
  }

  return -1;
}

/************* fetch_client_TCB() ****************/

// helper function

client_tcb_t *fetch_client_TCB(unsigned int ClientPort)
{

  int i;

  for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

    if (p_client_tcb_table[i] != NULL && p_client_tcb_table[i]->client_portNum == ClientPort) {

      return p_client_tcb_table[i];
    }
  }

  return NULL;
}

void sendBuffer_append_segment(client_tcb_t *table, segBuf_t *segmentBuffer)
{

  // since I used memset, all of the buffer structures variables are intialized to zero

  pthread_mutex_lock(table->bufMutex);

  if (table->sendBufHead == 0) {

    segmentBuffer->seg.header.seq_num = table->next_seqNum;

    segmentBuffer->sentTime = 0;

    table->next_seqNum = table->next_seqNum + segmentBuffer->seg.header.length;

    table->sendBufHead = segmentBuffer;

    table->sendBufunSent = segmentBuffer;

    table->sendBufTail = segmentBuffer;

  } else {

    segmentBuffer->seg.header.seq_num = table->next_seqNum;

    table->next_seqNum += segmentBuffer->seg.header.length;

    table->sendBufTail->next = segmentBuffer;

    table->sendBufTail = segmentBuffer;

    if (table->sendBufunSent == 0) {

      table->sendBufunSent = segmentBuffer;
    }
  }

  pthread_mutex_unlock(table->bufMutex);

}

/********* sendBuffer_sending() *****************/

// helper function

void sendBuffer_sending(client_tcb_t *table)
{
  pthread_mutex_lock(table->bufMutex);

  while (table->unAck_segNum < GBN_WINDOW && table->sendBufunSent != 0)
  {

    mnp_sendseg(network_connection, table->svr_nodeID, (seg_t *)table->sendBufunSent);

    struct timeval timer;

    gettimeofday(&timer, NULL);

    table->sendBufunSent->sentTime = timer.tv_sec * 1000 + timer.tv_usec;

    if (table->unAck_segNum == 0) {

      pthread_t timer;

      pthread_create(&timer, NULL, sendingBuffer_timer, (void *)table);
    }

    table->unAck_segNum++;

    if (table->sendBufunSent != table->sendBufTail) {

      table->sendBufunSent = table->sendBufunSent->next;

    } else {

      table->sendBufunSent = 0;
    }
  }

  pthread_mutex_unlock(table->bufMutex);
}


/******************** sendingBuffer_timer() ****************/

// helper function

void *sendingBuffer_timer(void *table)
{
  client_tcb_t *current_table = (client_tcb_t *)table;

  while (1) {

    select(0, 0, 0, 0, &(struct timeval) {.tv_usec = SENDBUF_POLLING_INTERVAL});

    struct timeval currentTime;

    gettimeofday(&currentTime, NULL);

    if (current_table->unAck_segNum == 0) {

      pthread_exit(NULL);

    } else if (current_table->sendBufHead->sentTime > 0 && current_table->sendBufHead->sentTime < currentTime.tv_sec * 1000000 + currentTime.tv_usec - DATA_TIMEOUT) {

      sendBuffer_timeout(current_table);
    }
  }
}

/***********  sendBuffer_timeout()*****************/

//helper function

void sendBuffer_timeout(client_tcb_t *table)
{
  pthread_mutex_lock(table->bufMutex);

  segBuf_t *buffer = table->sendBufHead;

  // go through unack segments
  for (int i = 0; i < table->unAck_segNum; i++) {

    // send the segments to MNP

    mnp_sendseg(network_connection, table->svr_nodeID, (seg_t *)buffer);

    struct timeval currentTime;

    gettimeofday(&currentTime, NULL);

    // set sentTime

    buffer->sentTime = currentTime.tv_sec * 10000 + currentTime.tv_usec;

    buffer = buffer->next;
  }

  pthread_mutex_unlock(table->bufMutex);
}

/*************** sendBufferRecvAck() *******************/

// helper function

void sendBufferRecvAck(client_tcb_t *table, unsigned int ack_seqnum)
{
  pthread_mutex_lock(table->bufMutex);


// if acknowldege number is bigger than the segbuffer sequence number than set buf tail to 0
  if (ack_seqnum > table->sendBufTail->seg.header.seq_num) {

    table->sendBufTail = 0;
  }

  segBuf_t *bufferHead = table->sendBufHead;

// while acknowlsrged sequence number is bigger than the sequence number

  while (bufferHead && bufferHead->seg.header.seq_num < ack_seqnum) {

    // send the recieved acknowldged segment

    table->sendBufHead = bufferHead->next;

    segBuf_t *temperary = bufferHead;

    bufferHead = bufferHead->next;

    free(temperary);

    table->unAck_segNum--;
  }

  pthread_mutex_unlock(table->bufMutex);
}


/******** sendBuffer_destroy() ******/

// helper function

void sendBuffer_destroy(client_tcb_t *table)
{

  // lock mutex

  pthread_mutex_lock(table->bufMutex);

  segBuf_t *bufer = table->sendBufHead;

  // clear the sent buffer

  while (bufer != table->sendBufTail) {

    segBuf_t *temp = bufer;

    bufer = bufer->next;

    free(temp);
  }

  // free tabble and set sendBuffer paramters to 0

  free(table->sendBufTail);

  table->sendBufunSent = 0;

  table->sendBufHead = 0;

  table->sendBufTail = 0;

  table->unAck_segNum = 0;

  pthread_mutex_unlock(table->bufMutex);
}


