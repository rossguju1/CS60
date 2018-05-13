/*
 * overlay/neighbortable.c: implementation of APIs for the neighbor 
 * table.  
 *
 * CS60, March 2018. 
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "neighbortable.h"
#include "../topology/topology.h"

/**************** local constants ****************/
#define TOPOLOGYFILE "../topology/topology.dat"

/**************** functions ****************/
// TODO: 
// 
// nt_create()
// 
// input: NONE
// output: return neighboor table

nbr_entry_t *nt_create() {

// define hostnames, address for IP
	int node_id1, node_id2;
	char host1[256];
	char host2[256];
	int link_cost;
	struct hostent *hptr1;
	struct hostent *hptr2;
	struct in_addr address1;
	struct in_addr address2;
	int My_node_id;
	char hostname[2048];
	int status;
	

	

	int nbrNum;
// get neighboors
	nbrNum = topology_getNbrNum();

// allocate memmory
  nbr_entry_t* nt = (nbr_entry_t*)malloc(nbrNum * sizeof(nbr_entry_t));

// get the domain name and save in char array
 status = gethostname(hostname, 2048);

 if (status != 0) {
 	printf("%s\n", "failed to gethostname");
 }

// get ID name of hostname
My_node_id = topology_getNodeIDfromname(hostname);

// open file topology.dat
	FILE *fp = fopen(TOPOLOGYFILE, "r");
	if (fp == NULL) {
		printf("%s \n", "couldnt open file ");
		return NULL;
	}

int idx = 0;
// get the domains and link costs
while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF ) {
	// get ID name
	node_id1 = topology_getNodeIDfromname(host1);
   	node_id2 = topology_getNodeIDfromname(host2);

   	hptr1 = gethostbyname(host1);
   	// get address IP in addr_t
  	memcpy(&address1, hptr1->h_addr_list[0], hptr1->h_length);
  	hptr2 = gethostbyname(host2);
  	memcpy(&address2, hptr2->h_addr_list[0], hptr2->h_length);

   	printf("node_id1~~> %d \n", node_id1);
   	printf("node_id2~~> %d \n", node_id2);

	// if local node ID is the same as one of the host domains, save the next node ID
   	if (node_id1 == My_node_id) {
   		nt[idx].nodeID = node_id2;
   		nt[idx].nodeIP = address2.s_addr;;
   		nt[idx].conn = -1;
   		idx++;

   		
   	} else if (node_id2 == My_node_id) {
   		nt[idx].nodeID = node_id1;
   		nt[idx].nodeIP = address1.s_addr;;
   		nt[idx].conn = -1;
   		idx++;
   		
   	} 
   	
}
		
// cloea file descriptor
fclose(fp);


	return nt;
}

// nt_destroy()
// clean up the malloc'd table
// input :neighborhood table
// output :none
void nt_destroy(nbr_entry_t *nt) {

	int i;
	// get number of neighbors
	int nbrNum = topology_getNbrNum();
	for (i = 0; i < nbrNum; i++) {
// if the connection was made
  if(nt[i].conn != -1) {
  	// clean up
  	close(nt[i].conn);
    free(nt);
    nt = NULL;
    return;
		}
	}
return;
}



// nt_addconn()
// input : neigborhood table, nodeID and connection socket descriptor
// output : none
int nt_addconn(nbr_entry_t *nt, int nodeID, int conn) {


  int i, nbrNumber;
  	// get number of neighbors

 nbrNumber = topology_getNbrNum();
  for(i = 0; i < nbrNumber; i++) {
  	if (nt[i].nodeID <= 0) {
  		// if node ID is empty
  		// go to next index
  		continue;
  	}
    if(nt[i].nodeID == nodeID) { 
      nt[i].conn = conn;
      // if ID matches the ID in the table add the connection
      printf("\t  nt_addconn ~~~> added sock to nodeid %d with sock %d\n\n", nodeID, conn);
      return 1;
    	}
  	}
  return -1;	
}
