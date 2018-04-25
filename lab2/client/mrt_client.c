/****
Lab2 
Ross Guju



***/



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
#include "mrt_client.h"

/**************** global variables ****************/
client_tcb_t *p_client_tcb_table[MAX_TRANSPORT_CONNECTIONS];
int overlay_connection;
pthread_mutex_t mutex;
pthread_cond_t cond;

/**************** functions ****************/

// TODO - mrt client initialization


void *segBuffer_timer(void* arg);
void send_buff(client_tcb_t *client_TCB);
void *buffer_timer(void* client);
void append_buffer(client_tcb_t *client_TCB, segBuf_t *buffer);
//int send_acknowledge_timer(seg_t *segment, client_tcb_t *client_TCB , unsigned int expect_state, unsigned int TimeOut);

client_tcb_t *fetch_TCB( seg_t* newSegment);

void mrt_client_init(int conn) {

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
    p_client_tcb_table[i] = NULL;
  }
  overlay_connection = conn;

  pthread_t seghandler_thread;
  pthread_create(&seghandler_thread, NULL, seghandler, (void *) 0);

}

// TODO - Create a client sock
int mrt_client_sock(unsigned int client_port) {
  int i;
  for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

    client_tcb_t *client_TCB;
    if (p_client_tcb_table[i] == NULL) {
      p_client_tcb_table[i] = (client_tcb_t*) malloc(sizeof(client_tcb_t));
      client_TCB =  p_client_tcb_table[i];
      client_TCB->bufMutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
      pthread_mutex_init(client_TCB->bufMutex, NULL);
      client_TCB->state = CLIENT_CLOSED;
      client_TCB->client_portNum = client_port;
      client_TCB->next_seqNum = 0;
      client_TCB->sendBufHead = NULL;
      client_TCB->sendBufunSent = NULL;
      client_TCB->sendBufTail = NULL;
      client_TCB->unAck_segNum = 0;
      return i;
    }
  }
  return -1;
}

/******* mrt_client_connect******/

int mrt_client_connect(int sockfd, unsigned int server_port) {
  int l;
  if (sockfd > MAX_TRANSPORT_CONNECTIONS || sockfd < 0) {
    perror("mrt_client_connect failed\n");
    return -1;
  }

// connect to the server by sending syn
  client_tcb_t *client_TCB = p_client_tcb_table[sockfd];
  seg_t *segment = (seg_t*) malloc(sizeof(seg_t));

  client_TCB->svr_portNum = server_port;
  segment->header.length = 0;
  segment->header.type = SYN;
  segment->header.src_port = client_TCB->client_portNum;
  segment->header.dest_port = client_TCB->svr_portNum;
  segment->header.seq_num = 0;
  mnp_sendseg(overlay_connection, segment);
  printf("For sending syn from client, the clients own port number is %d \n", client_TCB->client_portNum);



  client_TCB->state = CLIENT_SYNSENT;
// switch to synsent state
  for (int i = 0; i < SYN_MAX_RETRY; i++) {

    if (client_TCB->state == CLIENT_CONNECTED) {
      printf("%s\n", "Client connected\n\n\n\n" );
      mnp_sendseg(overlay_connection, segment);
      return 1;
    } else if (i == SYN_MAX_RETRY) {
      mnp_sendseg(overlay_connection, segment);
      return 1;
    }
    else {
      mnp_sendseg(overlay_connection, segment);
      continue;
    }
  }
  if (client_TCB->state == CLIENT_CONNECTED) {
    return 1;
  }

  return 1;
}


/******* mrt_client_send()******/

// TODO - Send data to a mrt server
int mrt_client_send(int sockfd, void *data, unsigned int length)
{

  segBuf_t *buffer;
  client_tcb_t *client_TCB;
  client_TCB = p_client_tcb_table[sockfd];


  if (sockfd > MAX_TRANSPORT_CONNECTIONS || sockfd < 0) {
    perror("mrt_client_send failed\n");
    return -1;
  }

  while (p_client_tcb_table[sockfd]->state != CLIENT_CONNECTED ) {
    sleep(1);
  }

  buffer = (segBuf_t *) malloc(sizeof(segBuf_t));

//pthread_mutex_lock(client_TCB->bufMutex);
  while (length > 0) {

//intialize the buffer

    buffer->seg.header.type = DATA;
    buffer->seg.header.seq_num = client_TCB->next_seqNum;
    buffer->seg.header.src_port = client_TCB->client_portNum;
    buffer->seg.header.dest_port = client_TCB->svr_portNum;
    buffer->sentTime = 0;
    buffer->next = NULL;


    if (length < MAX_SEG_LEN) {
      memcpy(buffer->seg.data, data, length);
      buffer->seg.header.length = length;
      client_TCB->next_seqNum = client_TCB->next_seqNum + length;


    }

    if (client_TCB->sendBufHead == NULL) {
      client_TCB->sendBufHead = buffer;
      client_TCB->sendBufTail = buffer;
      client_TCB->sendBufunSent = buffer;
// } else {
//      //client_TCB->sendBufunSent = buffer;
//    client_TCB->sendBufTail->next = buffer;
//    client_TCB->sendBufTail = buffer;


    }
//send buffer
    if (client_TCB->sendBufunSent == NULL ) {

      client_TCB->sendBufunSent = buffer;
      client_TCB->sendBufTail->next = buffer;
      client_TCB->sendBufTail = buffer;
    }
//pthread_mutex_unlock(client_TCB->bufMutex);
    send_buff(client_TCB);
    client_TCB->unAck_segNum = client_TCB->unAck_segNum + 1;

  }
//pthread_mutex_unlock(client_TCB->bufMutex);


  return 1;
}



// TODO - Disconnect from the server
int mrt_client_disconnect(int sockfd) {

  seg_t segment;
  segBuf_t *segBuffer;
  client_tcb_t *client_TCB = p_client_tcb_table[sockfd];


  segment.header.seq_num = client_TCB->next_seqNum;
  segment.header.type = FIN;
  segment.header.length = 0;
  segment.header.src_port = client_TCB->client_portNum;
  segment.header.dest_port = client_TCB->svr_portNum;
  client_TCB->state = CLIENT_FINWAIT;
//send_acknowledge_timer(&segment, client_TCB, CLIENT_CLOSED, FINSEG_TIMEOUT_NS);

//pthread_mutex_lock(client_TCB->bufMutex);
  while ((segBuffer = client_TCB->sendBufHead) != NULL) {

    client_TCB->sendBufHead = client_TCB->sendBufHead->next;
    free(segBuffer);
    segBuffer = client_TCB->sendBufHead;
  }
  client_TCB->state = CLIENT_CLOSED;
  client_TCB->unAck_segNum = 0;
  client_TCB->sendBufHead = NULL;
  client_TCB->sendBufTail = NULL;
  client_TCB->sendBufunSent = NULL;
  //pthread_mutex_unlock(client_TCB->bufMutex);


  return 1;
}

/**** mrt_client_close()******/

int mrt_client_close(int sockfd) {

  client_tcb_t *client_TCB = p_client_tcb_table[sockfd];

  if (client_TCB->state == CLIENT_CLOSED) {
    // clean up
    pthread_mutex_destroy(client_TCB->bufMutex);
    free(client_TCB->bufMutex);
    free(client_TCB);
    client_TCB = NULL;
    return 1;

  }

  return -1;
}


void *seghandler(void *arg)
{
  seg_t segment;    //Received segment
  seg_t segment_Received;    //Sending segment
  client_tcb_t *client_TCB;
  segBuf_t *buf;
// while we are sill getting messages
  while (mnp_recvseg(overlay_connection, &segment) != -1) {

    client_TCB = fetch_TCB(&segment);

    if (client_TCB == NULL) {

      continue;
    }
    switch (client_TCB->state) {
    // current state is closed
    case CLIENT_CLOSED:
      printf("Handling for closed\n");
      break;

    // recieved syn and now send
    case CLIENT_SYNSENT:
      printf("Handling for synsent\n");
      if (segment.header.type == SYNACK && client_TCB->svr_portNum == segment.header.src_port) {

        client_TCB->state = CLIENT_CONNECTED;
      }
      break;

    case CLIENT_CONNECTED:

      pthread_mutex_lock(client_TCB->bufMutex);


      if (segment.header.type == DATAACK && client_TCB->svr_portNum == segment.header.src_port) {

// send the buffer
        if (segment.header.seq_num > client_TCB->sendBufTail->seg.header.seq_num) {
          segBuf_t *segBufp;
          segBufp = client_TCB->sendBufHead;
          while (segBufp && segBufp->seg.header.seq_num < segment.header.seq_num ) {
            client_TCB->sendBufHead = segBufp->next;

            segBuf_t* temp = segBufp;
            segBufp = segBufp->next;
            free(temp);
            client_TCB->unAck_segNum--;
          }
          send_buff(client_TCB);
        }
      }

      pthread_mutex_unlock(client_TCB->bufMutex);
      break;
    case CLIENT_FINWAIT:
      if (segment.header.type == FINACK && client_TCB->svr_portNum == segment.header.src_port) {
        client_TCB->state = CLIENT_CLOSED;
        break;
      }
    }

  }
  return NULL;
}

/***** buffer_timer*****/

void *buffer_timer(void* client)
{

  client_tcb_t *client_TCB;
  client_TCB = (client_tcb_t*)client_TCB;
  segBuf_t *buf;
  int i;

  struct timespec req;


// go through and get the time sent
  for (;;) {

    req.tv_sec = 0;
    req.tv_nsec = 10000000;
    nanosleep(&req, NULL);

    if (client_TCB->sendBufHead == NULL || client_TCB->sendBufHead == client_TCB->sendBufunSent)
    {
      break;
    }

    i = time(NULL) - client_TCB->sendBufHead->sentTime;
    if (i > DATA_TIMEOUT)
    {


      buf = client_TCB->sendBufHead;
      while (buf != client_TCB->sendBufunSent)
      {
        mnp_sendseg(overlay_connection, &buf->seg);
        buf->sentTime = time(NULL);
        buf = buf->next;
      }
    }
  }
  return NULL;
}



/******* send_acknowloedgement_timer() *******/


int send_acknowledge_timer(seg_t *segment, client_tcb_t *client_TCB , unsigned int expect_state, unsigned int TimeOut)
{
  int max_syn_reached;
  struct timespec timer;

  int i = 1;
  for (i = 1; i < SYN_MAX_RETRY; i++) {
    {
      mnp_sendseg(overlay_connection, segment);

      pthread_mutex_lock(&mutex);

      clock_gettime(CLOCK_MONOTONIC, &timer);

      timer.tv_nsec = (timer.tv_nsec + TimeOut) % SYNSEG_TIMEOUT_NS;

      max_syn_reached = pthread_cond_timedwait(&cond, &mutex, &timer);


      pthread_mutex_unlock(&mutex);

      if  (max_syn_reached  == ETIMEDOUT) {
        continue;
      }

      if (client_TCB->state == expect_state) {

        return 1;

      }
    }
  }

  return -1;
}
/**** fecthed_TCB****/
client_tcb_t *fetch_TCB( seg_t* newSegment)
{

  client_tcb_t *client_TCB;

  int seg_portnum = newSegment->header.dest_port;
// get the data structure acdording to the socket file descriptor
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
    client_TCB = p_client_tcb_table[i];
    if (client_TCB == NULL) {
      continue;
    }

    if (client_TCB->client_portNum == seg_portnum) {
      return client_TCB;
    }

  }
  return NULL;

}

/********* send_buffer()********/
void send_buff(client_tcb_t *client_TCB)
{
  pthread_t ThreadID;

  int i;

  pthread_mutex_lock(client_TCB->bufMutex);
  for ((i = GBN_WINDOW - client_TCB->unAck_segNum); i > 0; i--) {
// while (client_TCB->sendBufunSent != NULL ){

    if (client_TCB->sendBufunSent != NULL) {
      mnp_sendseg(overlay_connection, &client_TCB->sendBufunSent->seg);
//sleep(1);
      client_TCB->sendBufunSent->sentTime = time(NULL);
//client_TCB->unAck_segNum = (client_TCB->unAck_segNum  + 1);

      if (client_TCB->sendBufunSent == client_TCB->sendBufHead) {

        if (pthread_create(&ThreadID, NULL, buffer_timer, (void*)client_TCB) != 0) {

        }

      }
      client_TCB->sendBufunSent = client_TCB->sendBufunSent->next;

    }

    if ( client_TCB->sendBufunSent == NULL) {
      break;
    }
  }
  pthread_mutex_unlock(client_TCB->bufMutex);
  return;
}

