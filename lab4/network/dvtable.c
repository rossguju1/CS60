/*
 * network/dvtable.c: implementation of functions for the distance
 * vector table used by the routing protocol of MNP.
 * Each node has an ON process maintaining a distance vector table
 * for the node that it is running on.
 *
 * Ross Guju
 *
 * CS60, March 2018.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../common/constants.h"
#include "../topology/topology.h"
#include "dvtable.h"




dv_entry_t *dv_entry_new(int sourceNode);

/** TODO:
 * This function creates a distance vector table dynamically.
 * A distance vector table contains the n+1 entries, where n is the
 * number of the neighbors of this node, and one is for this node
 * itself.
 * Each entry in distance vector table is a dv_t structure which
 * contains a source node ID and an array of N dv_entry_t structures
 * where N is the number of nodes in the network.
 * Each dv_entry_t contains a destination node ID and the cost from
 * the source node to this destination node.
 * The dvtable is initialized in this function.
 * The link costs from this node to its neighbors are initialized
 * using direct link cost retrieved from topology.dat. Other link
 * costs are initialized to INFINITE_COST.
 *
 * Return the dynamically created dvtable.
 */

dv_t *dvtable_create()
{

	// get needed array of nodes and IDS

	int nbrNum = topology_getNbrNum();

	int myNodeID = topology_getMyNodeID();
	int* nbrARRAY = topology_getNbrArray();



	dv_t* dv_table = (dv_t*)malloc((nbrNum + 1) * sizeof(dv_t)); //pointers to the headnodes

	// error checking

	if (!dv_table) {
		return NULL;
	}

	// go through and initialze array and make dv_entries

	for (int i = 0; i < (nbrNum); i++) {
		dv_table[i].nodeID = nbrARRAY[i];
		dv_table[i].dvEntry = dv_entry_new((int)nbrARRAY[i]);
	}

	//the last index is saved for the hostname ID
	dv_table[(nbrNum)].nodeID = myNodeID;
	dv_table[(nbrNum)].dvEntry = dv_entry_new(myNodeID);
	free(nbrARRAY);
	return dv_table;

}

/** TODO:
 * This function destroys a dvtable.
 * It frees all the dynamically allocated memory for the dvtable.
 */
void dvtable_destroy(dv_t *dvtable)
{
	int nbrNum = topology_getNbrNum();
	//int netnodeNum = topology_getNodeNum();

	for (int i = 0; i < (nbrNum + 1); i++) {
		free(dvtable[i].dvEntry);
	}
	free(dvtable);
}

/** TODO:
 * This function sets the link cost between two nodes in dvtable.
 * Return 1 if these two nodes are found in the table and the link
 * cost is set, otherwise -1.
 */
int dvtable_setcost(dv_t *dvtable, int fromNodeID, int toNodeID, unsigned int cost)
{
	int nbrNum = topology_getNbrNum();
	int netnodeNum = topology_getNodeNum();

	// go through table and set the cost from the node inputs

	for (int i = 0; i < (nbrNum + 1); i++) {
		dv_t current_dv = dvtable[i];
		if (fromNodeID == current_dv.nodeID) {
			for (int index = 0; index < netnodeNum; index++) {
				if (current_dv.dvEntry[index].nodeID == toNodeID) {
					current_dv.dvEntry[index].cost = cost;
					return 1;
				}
			}
		}
	}
	return -1;
}


/** TODO:
 * This function returns the link cost between two nodes in dvtable.
 * Return the link cost if these two nodes are found in dvtable,
 * otherwise INFINITE_COST.
 */
unsigned int dvtable_getcost(dv_t *dvtable, int fromNodeID, int toNodeID)
{
	int nbrNum = topology_getNbrNum();
	int netnodeNum = topology_getNodeNum();


	// printf("%s\n", "dvtable_getcost 1");

	// printf("fromNodeID~>%d  toNodeID~>%d \n", fromNodeID, toNodeID );
	for (int i = 0; i < (nbrNum); i++) {
		// printf("%s\n", "dvtable_getcost 2");

		if (fromNodeID == dvtable[i].nodeID) {
			// printf("%s\n", "dvtable_getcost 3");
			for (int index = 0; index < netnodeNum; index++) {
				// printf("%s\n", "dvtable_getcost 4");
				if (dvtable[i].dvEntry[index].nodeID == toNodeID) {
					// printf("%s\n", "dvtable_getcost 5");
					return dvtable[i].dvEntry[index].cost;
				}
			}
		}
	}

	return INFINITE_COST;
}


/** TODO:
 * This function prints out the contents of a dvtable.
 */
void dvtable_print(dv_t *dvtable)
{
	int nbrNum = topology_getNbrNum();
	int netnodeNum = topology_getNodeNum();
	printf("-------------dv table-------------\n");
	for (int i = 0; i < (nbrNum + 1); i++) {
		printf("~NBR Source NodeID~> :%d \n", dvtable[i].nodeID);
		for (int index = 0; index < netnodeNum; index++) {
			printf(" NBR Source NodeID :%d Destination NodeID :%d Src to destination cost link: %d \n\n", dvtable[i].nodeID, dvtable[i].dvEntry[index].nodeID, dvtable[i].dvEntry[index].cost);
		}
	}
	printf("----------------------------------\n");
}

/************* dv_entry_new() ************/

// helper function
//
// this function initializes dv_entries for every dvtable entry

dv_entry_t *dv_entry_new(int sourceNode)
{

	int netnodeNum = topology_getNodeNum();
	int myNodeID = topology_getMyNodeID();
	printf("dv_entry_new~~>netnodeNumber ~> %d\n", netnodeNum);

	int* nodeARRAY = topology_getNodeArray();
	/*
	for(int l=0; l < netnodeNum; l++){
		printf("dv_entry_new~~>nodeArray[i] %d\n", (int)nodeARRAY[l]);
	}
	*/
	dv_entry_t *entries = (dv_entry_t *)malloc(netnodeNum  * sizeof(dv_entry_t));
	if (!entries) {
		return NULL;
	}

	for (int i = 0; i < netnodeNum; i++) {

		entries[i].nodeID = (int)nodeARRAY[i];
		entries[i].cost = topology_getCost(sourceNode, (int)nodeARRAY[i]);
		printf("dv_entry_new making elements~>  entries[i].nodeID: %d. entries[i].cost%d \n", entries[i].nodeID, entries[i].cost);
	}
	free(nodeARRAY);

	return entries;
}
