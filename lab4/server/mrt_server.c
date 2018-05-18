/*
	CS60SU18

	lab4

	mrt_server.c

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
#include "mrt_server.h"

/********* local functions ************/

int create_TCB_table(unsigned int PORT);

server_tcb_t *fetch_server_TCB(unsigned int ServerPort);

void segment_recv(server_tcb_t *table, seg_t *segmentBuffer);

void *closewait(void *table);

int saveTobuffer(server_tcb_t *table, seg_t *segmentBuffer);

void data_received(server_tcb_t *table, seg_t *data_in);

/******* local variables **********/

server_tcb_t *p_server_tcb_table[MAX_TRANSPORT_CONNECTIONS];

int network_connection;

pthread_mutex_t mutex;

/************* functions ******************/


/**
 * This function initializes the TCB table marking all entries NULL.
 * It also initializes a global variable for the overlay TCP socket
 * descriptor "conn" used as input parameter for mnp_sendseg() and
 * mnp_recvseg(). Finally, the function starts the seghandler thread
 * to handle the incoming segments.
 *
 * Note: There is only one seghandler for the server side which
 * handles call connections for the client.
 *
 * @param conn - socket descriptor
 */
void mrt_server_init(int conn)
{

	int i;

	i = 0;

	// set p_server_tcb_table entries to NULL

	while (i < MAX_TRANSPORT_CONNECTIONS) {

		p_server_tcb_table[i] = NULL;

		i++;
	}

	network_connection = conn;

	pthread_t seghandler_thread;

	pthread_create(&seghandler_thread, NULL, seghandler, (void *) 0);
}


/**
 * This function looks up the server TCB table to find the first
 * NULL entry, and creates a new TCB entry using malloc() for that
 * entry. All fields in the TCB are initialized, e.g., TCB state is
 * set to CLOSED and the server port set to the function call
 * parameter server port. The TCB table entry index should be
 * returned as the new socket ID to the client and be used to
 * identify the connection on the client side. If no entry in the
 * TCB table is available the function returns -1.
 *
 * @param port - port number
 * @return 1 for success; -1 for failure
 */
int mrt_server_sock(unsigned int port)
{

	int server_socket;

	server_socket = create_TCB_table(port);

	// error check

	if (server_socket < 0) {

		printf("%s\n\n", "server transfer control block table is full");

		return -1;
	}

	char* received_buffer = (char*)malloc(sizeof(RECEIVE_BUF_SIZE));

	pthread_mutex_t *recvBuf_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

	// intialize mutex

	pthread_mutex_init(recvBuf_mutex, NULL);

	server_tcb_t *TCB_table = p_server_tcb_table[server_socket];

	TCB_table->bufMutex = recvBuf_mutex;

	TCB_table->state = SVR_CLOSED;

	TCB_table->svr_nodeID = topology_getMyNodeID();

	TCB_table->usedBufLen = 0;

	TCB_table->recvBuf = received_buffer;

	return server_socket;
}




/**
 * This function gets the TCB pointer using the sockfd and changes
 * the state of the connection to LISTENING. It then starts a timer
 * to "busy wait" until the TCB's state changes to CONNECTED
 * (seghandler does this when a SYN is received). It waits in an
 * infinite loop for the state transition before proceeding and to
 * return 1 when the state change happens, dropping out of the busy
 * wait loop. You can implement this blocking wait in different
 * ways, if you wish.
 *
 * @param sockfd - socket file descriptor
 * @return 1 for successful change; -1 for failure
 */
int mrt_server_accept(int sockfd)
{

	// get the p_server_tcb_table

	server_tcb_t *TCB_table = p_server_tcb_table[sockfd];

	// use FSM checking the state of the TCB_table

	switch (TCB_table->state) {

	case SVR_CLOSED:

		// change states

		TCB_table->state = SVR_LISTENING;

		while (1) {

			if (TCB_table->state == SVR_CONNECTED) {

				printf("%s\n", "server is connected");

				break;

			} else {

				// use select function to poll for accept_polling_ interval

				select(0, 0, 0, 0, &(struct timeval) {.tv_usec = ACCEPT_POLLING_INTERVAL});
			}
		}

		return 1;

	case SVR_CONNECTED:

		return -1;

	case SVR_LISTENING:

		return -1;

	case SVR_CLOSEWAIT:

		return -1;
	}

	return -1;
}






/**
 * Receive data to a MRT client. Recall this is a unidirectional
 * transport where DATA flows from the client to the server.
 * Signaling/control messages such as SYN, SYNACK, etc. flow in both
 * directions.
 *
 * @param sockfd - socket file descriptor
 * @param buf - receiving buffer
 * @param length - buffer length
 * @return 1 for success; -1 for failure
 */
int mrt_server_recv(int sockfd, void *buf, unsigned int length)
{

	// get server_TCB_table

	server_tcb_t *TCB_table = p_server_tcb_table[sockfd];

	// FSM to check the TCB_table states

	switch (TCB_table->state) {

	case SVR_CONNECTED:

		while (1) {

			// if in connected state, get bufferTo send to MNP

			char* bufferToSend;

			if (length > TCB_table->usedBufLen) {

				//  if the length is bigger than the current usedBuflen
				//  then sleep

				sleep(RECVBUF_POLLING_INTERVAL);

			} else {

				// mutex lock

				pthread_mutex_lock((TCB_table->bufMutex));

				// initialize bufferToSned

				bufferToSend = (char*) buf;

				// copy received buffer into buffeter to send

				memcpy(bufferToSend, TCB_table->recvBuf, length);

				// printf("buffer ~>%s\n", bufferToSend);

				memcpy(TCB_table->recvBuf, TCB_table->recvBuf + length, TCB_table->usedBufLen - length);

				// decrease the usedBuflen

				TCB_table->usedBufLen = TCB_table->usedBufLen - length;

				pthread_mutex_unlock(TCB_table->bufMutex);

				break;
			}
		}

		return 1;

	case SVR_CLOSEWAIT:

		return -1;

	case SVR_LISTENING:

		return -1;

	case SVR_CLOSED:

		return -1;

	default:

		return -1;
	}
}


/**
 * This function calls free() to free the TCB entry. It marks that
 * entry in TCB as NULL and returns 1 if succeeded (i.e., was in the
 * right state to complete a close) and -1 if fails (i.e., in the
 * wrong state).
 *
 * @param sockfd - socket file descriptor
 * @return 1 for success; -1 for failure
 */
int mrt_server_close(int sockfd)
{

	if (sockfd < 0) {

		printf("something went wrong.");

		return -1;
	}
	server_tcb_t *server_TCB = p_server_tcb_table[sockfd];

	switch (server_TCB->state) {

	case (SVR_LISTENING):

//close in the listening state

		printf("current state is not SVR_CLOSED\n");

		printf("Changing to SRV_CLOSED\n");

		return -1;

	case (SVR_CONNECTED):

//close in the sever connect state

		printf("sockfd is in connecged state\n");

		printf("Changing to SRV_CLOSED\n");

		return -1;

	case (SVR_CLOSED):

		// clean up since we are in closed state

		pthread_mutex_destroy(server_TCB->bufMutex);

		free(server_TCB->recvBuf);

		free(server_TCB->bufMutex);

		free(server_TCB);

		server_TCB->recvBuf = NULL;

		server_TCB->bufMutex = NULL;

		server_TCB = NULL;

		return 1;
	default:

		printf("Changing current state to SRV_CLOSED\n");

		return -1;
	}

	return 1;
}


/**
 * This is a thread started by mrt_server_init(). It handles all the
 * incoming segments from the client. The design of seghanlder is an
 * infinite loop that calls mnp_recvseg(). If mnp_recvseg() fails
 * then the overlay connection is closed and the thread is
 * terminated. Depending on the state of the connection when a
 * segment is received (based on the incoming segment) various
 * actions are taken. See the server FSM for more details.
 *
 * @param arg - arguments for the thread
 * @return 1 for success; -1 for failure
 */
void *seghandler(void *arg)
{

	// declare variables

	int source_nodeID;

	server_tcb_t *TCB_table;

	seg_t segmentBuffer;

	while (1) {

		// if something goes wrong when recieving the segmentbuffer close the nextwork and exit the pthread

		if (mnp_recvseg(network_connection, &source_nodeID, &segmentBuffer) < 0) {

			close(network_connection);

			pthread_exit(NULL);
		}

		// fetch the tcb_table according to the destination portnumber

		TCB_table = fetch_server_TCB(segmentBuffer.header.dest_port);

		// check FSM

		switch (TCB_table->state) {

		case SVR_LISTENING:

			printf("%s\n", "SVR_LISTENING");

			TCB_table->client_nodeID = source_nodeID;

			TCB_table->client_portNum = segmentBuffer.header.src_port;

			// printf("TCB_table->client_nodeID => %d TCB_table->client_portNum => %d", TCB_table->client_nodeID, TCB_table->client_portNum);

			// use segment_recv function for header.type == SYN and FIN

			segment_recv(TCB_table, &segmentBuffer);

			// change state

			TCB_table->state = SVR_CONNECTED;

			break;

		case SVR_CONNECTED:

			printf("%s\n", "SVR_CONNECTED");

			// double chekc if the portnumber and node ID of client match the segment and TCB_table

			if (TCB_table->client_portNum == segmentBuffer.header.src_port && TCB_table->client_nodeID == source_nodeID) {

				if (segmentBuffer.header.type == SYN) {

					segment_recv(TCB_table, &segmentBuffer);

				} else if (segmentBuffer.header.type == DATA) {

					// recieve data from segmentBuffer

					data_received(TCB_table, &segmentBuffer);

				} else if (segmentBuffer.header.type == FIN) {

					// change state to CLOSEWAIT if FIN

					TCB_table->state = SVR_CLOSEWAIT;

					// start new waiting thread for closewait

					pthread_t wait;

					pthread_create(&wait, NULL, closewait, (void*)TCB_table);

					printf("%s\n", "server side fin received");

					segment_recv(TCB_table, &segmentBuffer);

					printf("%s\n", "serverside in close wait now");
				}
			}

			break;

		case SVR_CLOSEWAIT:

			printf("%s\n", "SVR_CLOSEWAIT");

			if (segmentBuffer.header.type == FIN) {

				segment_recv(TCB_table, &segmentBuffer);

			} else {

				printf("%s\n\n", "server closewait state but no fin seg recieved");
			}

		case SVR_CLOSED:

			printf("%s\n", "SVR_CLOSED");

			break;

		default:

			break;
		}
	}
}

/****** create_TCB_table() *****************************/

int create_TCB_table(unsigned int PORT)
{
	int i;

	// initialize p_server_tcb_table

	for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

		if (p_server_tcb_table[i] == NULL) {

			p_server_tcb_table[i] = (server_tcb_t*) malloc(sizeof(server_tcb_t));

			p_server_tcb_table[i]->svr_portNum = PORT;

			return i;

		} else if (p_server_tcb_table[i] != NULL) {

			continue;
		}
	}

	return -1;
}


/*********** fetch_server_TCB() ****************/

server_tcb_t *fetch_server_TCB(unsigned int ServerPort)
{

	int i;

	// go thgrough table and find the server portnumber

	for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

		if (p_server_tcb_table[i] != NULL && p_server_tcb_table[i]->svr_portNum == ServerPort) {

			return p_server_tcb_table[i];
		}
	}

	return NULL;
}

/********** segment_recv() ********************/

void segment_recv(server_tcb_t *table, seg_t * segmentBuffer)
{
	// if segmentBuffer->header.type == SYN

	// header.type is SYN
	//
	// then set headertype to synack and send biffer to MNP

	if (segmentBuffer->header.type == SYN) {

		seg_t synacknowledge;

		table->expect_seqNum = segmentBuffer->header.seq_num;

		memset(&synacknowledge, 0, sizeof(seg_t));

		synacknowledge.header.type = SYNACK;

		synacknowledge.header.dest_port = table->client_portNum;

		synacknowledge.header.src_port = table->svr_portNum;

		synacknowledge.header.length = 0;

		mnp_sendseg(network_connection, table->client_nodeID, &synacknowledge);

		printf("%s\n", "synack sent from segment_recv from server side");


		// if header.type is FIN change header.type to FINACK and send to MNP

	} else if (segmentBuffer->header.type == FIN) {

		seg_t FIN_ACKNOWLEDGE;

		memset(&FIN_ACKNOWLEDGE, 0, sizeof(seg_t));

		FIN_ACKNOWLEDGE.header.type = FINACK;

		FIN_ACKNOWLEDGE.header.dest_port = table->client_portNum;

		FIN_ACKNOWLEDGE.header.src_port = table->svr_portNum;

		FIN_ACKNOWLEDGE.header.length = 0;

		mnp_sendseg(network_connection, table->client_nodeID, &FIN_ACKNOWLEDGE);

		printf("%s\n", "finack sent from segment_recv from server side");

	}
}

/************** closewait()*************/

void *closewait(void *table)
{
	server_tcb_t *TCB_table = (server_tcb_t *)table;

	sleep(CLOSEWAIT_TIME);

	pthread_mutex_lock(TCB_table->bufMutex);

	// lock and set usedbuglen to 0

	TCB_table->usedBufLen = 0;

	pthread_mutex_unlock(TCB_table->bufMutex);

	// change state

	TCB_table->state = SVR_CLOSED;

	pthread_exit(NULL);
}

/*************** data_received() *************/

void data_received(server_tcb_t *table, seg_t *data_in)
{
	if (data_in->header.seq_num == table->expect_seqNum) {

		if (saveTobuffer(table, data_in) < 0) {

			return;
		}
	}

	seg_t DATA_ACKNOWLEDGE;

	memset(&DATA_ACKNOWLEDGE, 0, sizeof(seg_t));

	DATA_ACKNOWLEDGE.header.type = DATAACK;

	DATA_ACKNOWLEDGE.header.dest_port = table->client_portNum;

	DATA_ACKNOWLEDGE.header.src_port = table->svr_portNum;

	DATA_ACKNOWLEDGE.header.length = 0;

	DATA_ACKNOWLEDGE.header.ack_num = table->expect_seqNum;

	mnp_sendseg(network_connection, table->client_nodeID, &DATA_ACKNOWLEDGE);

	printf("%s\n", "dataack sent from segment_recv from server side");

}

/******************* saveTobuffer() *********************/

int saveTobuffer(server_tcb_t *table, seg_t *segmentBuffer)
{

	// if the segment header.length + usedBuflength, is biggfer than the max buf size

	if (RECEIVE_BUF_SIZE > segmentBuffer->header.length + table->usedBufLen) {

		pthread_mutex_lock(table->bufMutex);

		// copy recvbuffer then adjust usedbuflen and expected seq_num

		memcpy(&table->recvBuf[table->usedBufLen], segmentBuffer->data, segmentBuffer->header.length);

		table->usedBufLen = table->usedBufLen + segmentBuffer->header.length;

		table->expect_seqNum = segmentBuffer->header.length + segmentBuffer->header.seq_num;

		pthread_mutex_unlock(table->bufMutex);

		return 1;

	} else {

		return -1;
	}


}

