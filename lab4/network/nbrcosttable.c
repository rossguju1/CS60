/*
 * network/nbrcosttable.c: implement all operations for the neighbor 
 * cost table used by the routing protocol of MNP. 
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
#include "nbrcosttable.h"
#include "../common/constants.h"
#include "../topology/topology.h"


/** TODO: 
 * This function creates a neighbor cost table dynamically
 * and initializes the table with all its neighbors' node IDs and 
 * direct link costs. The neighbors' node IDs and direct link costs 
 * are retrieved from topology.dat file.
 *
 * Return the pointer to the table
 */
nbr_cost_entry_t *nbrcosttable_create() 
{
 int nbrNum = topology_getNbrNum();

printf("%s\n", "test1");
 int* nbrARRAY = topology_getNbrArray();
printf("%s\n", "test2");
 int myNodeID = topology_getMyNodeID();

printf("%s\n", "test3");

	nbr_cost_entry_t *nbrtable = (nbr_cost_entry_t *)malloc(nbrNum * sizeof(nbr_cost_entry_t));
 
	if (!nbrtable) {
		return NULL;
	}


printf("%s\n", "loop");
	for (int i = 0; i < nbrNum; i++) {
		printf("%s\n", "each iteration of loop");
		nbrtable[i].nodeID = nbrARRAY[i];
		nbrtable[i].cost = topology_getCost(myNodeID, nbrARRAY[i]);

	}
	printf("%s\n", "freeing nbrARRAY");
	free(nbrARRAY);

	return nbrtable; 
}

/** TODO: 
 * This function destroys a neighbor cost table.
 * It frees all the dynamically allocated memory for the neighbor 
 * cost table.
 */
void nbrcosttable_destroy(nbr_cost_entry_t *nct) 
{
free(nct);
}

/** TODO: 
 * This function is used to get the direct link cost from neighbor 
 * specified by its node ID.
 * Return the direct link cost if the neighbor is found in the 
 * table, otherwise INFINITE_COST if the node is not found. 
 */
unsigned int nbrcosttable_getcost(nbr_cost_entry_t *nct, int nodeID) 
{	
	 int nbrNum = topology_getNbrNum();
	for(int i = 0; i < nbrNum; i++){
		if (nct[i].nodeID == nodeID) {
			return nct[i].cost;
		}
	}
	return INFINITE_COST;
}


/** TODO: 
 * This function prints out the contents of a neighbor cost table.
 */
void nbrcosttable_print(nbr_cost_entry_t *nct) 
{
	int nbrNum = topology_getNbrNum();

	printf("-------------nbrcost table------------\n");
	for(int i = 0; i < nbrNum; i++) {
		printf("Neighber node :%d link Cost: %d \n\n", nct[i].nodeID, nct[i].cost);
	}
	printf("--------------------------------------\n");
}











