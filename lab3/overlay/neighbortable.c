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
// 1) get number of neighbors
// 2) allocate neighbor table
// 3) open topology file
//      for each line of file
//        get nodeId from name1
//        get hostname
//        set nt[idx].nodeIP
//        nt[idx].conn = -1;
//        idx++
//
//        get nodeId from name2
//        get hostname
//        set nt[idx].nodeIP
//        nt[idx].conn = -1;
//        idx++
// 4) return nt
nbr_entry_t *nt_create() {

	return 0;
}

// TODO: 
// 1) get number of neighbors
// 2) for each neighbord
//      if(nt[i].conn != -1)
//        close the connection
// 3) free(nt)
void nt_destroy(nbr_entry_t *nt) {

}


// TODO: 
// 1) get number of neighbors
// 2) for each neighbor
//      if nodeID == nt[i].nodeID
//        nt[i].conn = conn
//        return 1
int nt_addconn(nbr_entry_t *nt, int nodeID, int conn) {
	return 1;
}
