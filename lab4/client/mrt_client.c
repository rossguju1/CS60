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

// enum client_state_t {
// 	CLIENT_CLOSED,
// 	CLIENT_SYNSENT,
// 	CLIENT_CONNECTED,
// 	CLIENT_FINWAIT
// };

// // unit to store segments in send buffer.
// typedef struct segBuf {
// 	seg_t seg;
// 	unsigned int sentTime;
// 	struct segBuf* next;
// } segBuf_t;


// // client transport control block (TCB), containing information
// // of one overlay connection at the client side.
// typedef struct client_tcb {
// 	unsigned int svr_nodeID;        //node ID of server, obtained by  topology_getNodeIDfromname()
// 	unsigned int svr_portNum;       //port number of server
// 	unsigned int client_nodeID;     //node ID of client, obtained by  topology_getNodeIDfromname()
// 	unsigned int client_portNum;    //port number of client
// 	unsigned int state;     				//state of client

// 	// fields below are used for data transmission
// 	unsigned int next_seqNum;
// 	pthread_mutex_t* bufMutex;
// 	segBuf_t* sendBufHead;
// 	segBuf_t* sendBufunSent;
// 	segBuf_t* sendBufTail;
// 	unsigned int unAck_segNum;
// } client_tcb_t;




/**************** global variables ****************/
client_tcb_t *p_client_tcb_table[MAX_TRANSPORT_CONNECTIONS];

int network_connection;

pthread_mutex_t mutex;

pthread_cond_t cond;

/************* functions ********************/



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
// 	unsigned int svr_nodeID;        //node ID of server, obtained by  topology_getNodeIDfromname()
// 	unsigned int svr_portNum;       //port number of server
// 	unsigned int client_nodeID;     //node ID of client, obtained by  topology_getNodeIDfromname()
// 	unsigned int client_portNum;    //port number of client
// 	unsigned int state;     				//state of client

// 	// fields below are used for data transmission
// 	unsigned int next_seqNum;
// 	pthread_mutex_t* bufMutex;
// 	segBuf_t* sendBufHead;
// 	segBuf_t* sendBufunSent;
// 	segBuf_t* sendBufTail;
// 	unsigned int unAck_segNum;
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

int mrt_client_connect(int socked, int nodeID, unsigned int server_port)
{


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






}


/**
 * This is a thread started by mrt_client_init(). It handles all the
 * incoming segments from the server. The design of seghanlder is an
 * infinite loop that calls mnp_recvseg(). If snp_recvseg() fails
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




	return NULL;
}


int create_TCB_table(unsigned int PORT)
{
	int i;

	for (i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {

		if (p_client_tcb_table[i] == NULL) {

			p_client_tcb_table[i] = (client_tcb_t*) malloc(sizeof(client_tcb_t));

			p_client_tcb_table[i]->client_portNum = PORT;

			return i;

		} else if (p_client_tcb_table[i] != NULL && p_client_tcb_table[i]->client_portNum == PORT) {

			continue;
		}
	}

	return -1;
}

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

