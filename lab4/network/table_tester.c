#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include <strings.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <stdbool.h>
//#include "../topology/topology.h"
//#include "dvtable.h"
//#include "nbrcosttable.h"
//#include "routing_table.h"
//#include "../common/pkt.h"
//#include "../common/constants.h"
//#include "../common/seg.h"

#define TOPOLOGYFILE "../topology/topology.dat"
#define INFINITE_COST 999
#define MAX_ROUTINGTABLE_SLOTS 10


// typedef struct routingtable_entry {
// 	int destNodeID;	// destination node ID
// 	int nextNodeID;	// next-hop node ID to forward the packet
// 	struct routingtable_entry *next; // pointer to the next routingtable_entry_t in the same routing table slot
// } routingtable_entry_t;



// typedef struct set {
//  struct routingtable_entry *head;       // head of the set
// } set_t;


// typedef struct routing_table {
// 	struct set **table;		//*hash[MAX_ROUTINGTABLE_SLOTS];
// } routingtable_t;

typedef struct distancevectorentry {
	int nodeID;        	// destination nodeID
	unsigned int cost;	// cost to the destination
} dv_entry_t;


/* A distance vector table contains the n+1 dv_t entries,
 * where n is the number of the neighbors of this node,
 * and the rest one is for this node itself. */
typedef struct distancevector {
	int nodeID;        		// source nodeID
	dv_entry_t *dvEntry; 	// an array of N dv_entry_ts, each contains the destination node ID and the cost to the destination from the source node. N is the total number of nodes in the network.
} dv_t;




int topology_getNodeIDfromname(char* hostname);
int topology_getMyNodeID();
int topology_getNbrNum();

int topology_getNodeNum();

int* topology_getNodeArray();
void topology_delete_NodeArray(int* nodeArray);

int* topology_getNbrArray();
unsigned int topology_getCost(int fromNodeID, int toNodeID);

dv_t *dvtable_create();
void dvtable_destroy(dv_t *dvtable);
int dvtable_setcost(dv_t *dvtable, int fromNodeID, int toNodeID, unsigned int cost);
unsigned int dvtable_getcost(dv_t *dvtable, int fromNodeID, int toNodeID);
void dvtable_print(dv_t *dvtable);
dv_entry_t *dv_entry_new(int sourceNode);







// set_t *set_new(void);

// routingtable_entry_t *routing_entry_node_new(int destinationID, int nextID);

// bool set_insert(set_t *set, int destinationID, int nextID);

// void set_delete(set_t *set);

// bool hashtable_insert(routingtable_t *rt, int nodeID, int nextID);

// void hashtable_delete(routingtable_t *rt);

// int hashtable_find(routingtable_t *rt, int destID);

// int set_find(set_t *set, int destID);

// int makehash(int node);
// routingtable_t *routingtable_create();

// void routingtable_destroy(routingtable_t *routing_table);

// void routingtable_setnextnode(routingtable_t *routing_table, int destNodeID, int nextNodeID);

// void routingtable_print(routingtable_t *routing_table);

// int routingtable_getnextnode(routingtable_t *routing_table, int destNodeID);




int main(){
/*
printf("%s\n\n", "test1: nbrcosttable");
printf("%s\n", "~>___________________________<~");

printf("%s\n", "TEST1: segfault 1?");

nbr_cost_entry_t *costtable = nbrcosttable_create();

//nbrcosttable_create();
printf("%s\n", "TEST1: segfault 2?");

nbrcosttable_print(costtable);

printf("%s\n", "TEST1: segfault 3?");


nbrcosttable_destroy(costtable);

printf("%s\n", "TEST1: segfault 4?");

printf("%s\n\n", "test2: routing_table");
printf("%s\n", "~>___________________________<~");

printf("%s\n", "TEST2: segfault 1?");

routingtable_t *routeTable = routingtable_create();

printf("%s\n", "TEST2: segfault 2?");

printf("%s\n", "routingtable_setnextnode(routeTable, 4, 5);");
routingtable_setnextnode(routeTable, 4, 5);


printf("%s\n", "TEST2: segfault 3?");

printf("%s\n", "routingtable_setnextnode(routeTable, 4, 6);");
routingtable_setnextnode(routeTable, 4, 6);

printf("%s\n", "TEST2: segfault 4?");

printf("%s\n", "routingtable_setnextnode(routeTable, 4, 7);");
routingtable_setnextnode(routeTable, 4, 7);

printf("%s\n", "TEST2: segfault 5?");

printf("%s\n", "routingtable_setnextnode(routeTable, 13, 1);");
routingtable_setnextnode(routeTable, 13, 1);

printf("%s\n", "TEST2: segfault 6?");
printf("%s\n", "routingtable_setnextnode(routeTable, 13, 3);");
routingtable_setnextnode(routeTable, 13, 3);

printf("%s\n", "TEST2: segfault 7?");
printf("%s\n", "routingtable_setnextnode(routeTable, 13, 4);");
routingtable_setnextnode(routeTable, 13, 4);

printf("%s\n", "TEST2: segfault 8?");
printf("%s\n", "routingtable_getnextnode(routeTable, 13)");

printf("Destination node ID is 13: and next hope is %d \n\n", routingtable_getnextnode(routeTable, 13));

printf("%s\n", "TEST2: segfault 9?");
printf("%s\n", "routingtable_getnextnode(routeTable, 4)");
printf("Destination node ID is 4: and next hope is %d \n\n", routingtable_getnextnode(routeTable, 4));

printf("%s\n", "TEST2: segfault 11?");

printf("%s\n", "routingtable_setnextnode(routeTable, 1, 12);");

routingtable_setnextnode(routeTable, 1, 12);
printf("%s\n", "routingtable_setnextnode(routeTable, 2, 11)");
routingtable_setnextnode(routeTable, 2, 11);
printf("%s\n", "routingtable_setnextnode(routeTable, 15, 17)");
routingtable_setnextnode(routeTable, 15, 17);
printf("%s\n", "routingtable_setnextnode(routeTable, 6, 21)");
routingtable_setnextnode(routeTable, 6, 21);
printf("%s\n", "routingtable_setnextnode(routeTable, 6, 12)");
routingtable_setnextnode(routeTable, 6, 12);

printf("%s\n", "TEST2: segfault 12?");

routingtable_print(routeTable);

printf("%s\n", "TEST2: segfault 13?");

routingtable_destroy(routeTable);

printf("%s\n", "TEST2: segfault 14?");
*/
printf("%s\n\n", "test3: dvtable");
printf("%s\n", "~>___________________________<~");

printf("%s\n", "TEST3: segfault 1?");

dv_t *dv_Table = dvtable_create();

printf("%s\n", "TEST3: segfault 2?");

dvtable_print(dv_Table);

printf("%s\n", "TEST3: segfault 3?");

printf("node ID from 115 to 87 is: %d\n\n", dvtable_getcost(dv_Table, 115, 87));

printf("%s\n", "TEST3: segfault 4?");
printf("%s\n", "dvtable_setcost(dv_Table, 115, 87, 17);");

dvtable_setcost(dv_Table, 115, 87, 17);

printf("%s\n", "TEST3: segfault 5?");

printf("node ID from 115 to 87 is: %d\n\n", dvtable_getcost(dv_Table, 115, 87));

printf("%s\n", "TEST3: segfault 6?");

dvtable_print(dv_Table);

printf("%s\n", "TEST3: segfault 7?");

dvtable_destroy(dv_Table);

printf("%s\n", "TEST3: segfault 8?");

printf("%s\n", "PASSED SEGFAULT TEST");

return 0;
}

// set_t *set_new(void);

// routingtable_entry_t *routing_entry_node_new(int destinationID, int nextID);

// bool set_insert(set_t *set, int destinationID, int nextID);

// void set_delete(set_t *set);

// bool hashtable_insert(routingtable_t *rt, int nodeID, int nextID);

// void hashtable_delete(routingtable_t *rt);

// int hashtable_find(routingtable_t *rt, int destID);

// int set_find(set_t *set, int destID);

/*********************************************************
 * Global types
 * ******************************************************/
/*
 typedef struct routing_table {
 int num_slots; // length of pointer table
 struct set **table; // hashtable
} routingtable_t;
*/
/** 
 * This is the hash function used the by the routing table
 * It takes the hash nodeID - destination node ID as input,
 * and returns the hash value - slot number for this destination 
 * node ID.
 */
// int makehash(int node) 
// {
// 	return node % MAX_ROUTINGTABLE_SLOTS;
// }

// /** TODO: 
//  * This function creates a routing table dynamically.
//  * All the entries in the table are initialized to NULL pointers.
//  * Then for all the neighbors with direct links, create a routing 
//  * entry with the neighbor itself as the next hop node, and insert 
//  * this routing entry into the routing table.
//  *
//  * Return The dynamically created routing table structure.
//  */
// routingtable_t *routingtable_create() 
// {


// 	 int i; // counter for loop
//         routingtable_t *rt = malloc(sizeof(routingtable_t)); // actual hashtable
//         if (!rt) {
//             return NULL;
//         }

//         rt->table = calloc(MAX_ROUTINGTABLE_SLOTS, sizeof(set_t) ); //pointers to the headnodes

//         for (i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
//                 //create an empty set for all indexes in table
//                 rt->table[i] = set_new();
//         }

//         return rt;
// }

// /** TODO: 
//  * This function destroys a routing table.
//  * All dynamically allocated data structures for this routing table are freed. 
//  */
// void routingtable_destroy(routingtable_t *routing_table) 
// {

// 		hashtable_delete(routing_table);

// }

// /** TODO: 
//  * This function updates the routing table using the given 
//  * destination node ID and next hop's node ID.
//  * If the routing entry for the given destination already exists, 
//  * update the existing routing entry.
//  * If the routing entry of the given destination is not there, add 
//  * one with the given next node ID.
//  * Each slot in the routing table contains a linked list of routing 
//  * entries due to conflicting hash nodeIDs (different hash nodeIDs - 
//  * destination node ID - may have the same hash value (slot entry 
//  * number).
//  * To add an routing entry to the hash table:
//  * First use the hash function makehash() to get the slot number in 
//  * which this routing entry should be stored.
//  * Then append the routing entry to the linked list in that slot. 
//  */
// void routingtable_setnextnode(routingtable_t *routing_table, int destNodeID, int nextNodeID) 
// {
//   	if (!(hashtable_insert(routing_table, destNodeID, nextNodeID))){
//   		printf(" failed to insert routing entry in table\n");
//   	}


// }

// /** TODO: 
//  * This function looks up the destNodeID in the routing table.
//  * Since routing table is a hash table, this opeartion has O(1) time complexity.
//  * To find a routing entry for a destination node, first use the 
//  * hash function makehash() to get the slot number and then go 
//  * through the linked list in that slot to search for the routing 
//  * entry.
//  * Return nextNodeID if the destNodeID is found, else -1. 
//  */
// int routingtable_getnextnode(routingtable_t *routing_table, int destNodeID) 
// {
// 		int nextHop = hashtable_find(routing_table, destNodeID);

// 		if (nextHop >= 0) {
// 			return nextHop;
// 		}
		
// 	return -1;
// }

// /** TODO: 
//  * This function prints out the contents of the routing table. 
//  */
// void routingtable_print(routingtable_t *rt) {
// printf("-------------routing table------------\n");

// 	 for (int i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
// 	 	printf("ROUTING TABLE SLOT number: %d \n\n", i);
// 	  	if (rt->table[i] != NULL) {
// 	  		routingtable_entry_t *currentNode = rt->table[i]->head;
//         while ( currentNode != NULL ){  //iterate through all costs
//            printf("make sure that this set has same destination ID: %d ~> Next Node ID %d\n", currentNode->destNodeID, currentNode->nextNodeID);
//            currentNode = currentNode->next;
//           }
// 	 	 }
// 		}
// printf("--------------------------------------\n");
// }




// set_t *set_new(void)
// {
//         set_t *set = malloc(sizeof(set_t)); // allocate memory to *set pointer

//         if ( set == NULL ) {
//                 return NULL;       //error allocating memory 
//         } else {
//                 set->head = NULL;  // set first node head to be NULL
//                 return set;
//         }
// }



// bool set_insert(set_t *set, int destinationID, int nextID) {
//   if (set != NULL && nextID > 0 && destinationID > 0) {
//     // allocate a new node to be added to the list
//       routingtable_entry_t *new = routing_entry_node_new(destinationID, nextID);
//       if (new == NULL){
//         return false; //return false if new node is not created
//       }

//       if (set->head == NULL) {  //If set has no costs
//         set->head = new;  //Make the new Node the head of the set 
//       } else {
//         routingtable_entry_t *lastNode = set->head;
//         new->next = NULL;
//         //compare nodeID for head
//         while (lastNode->next != NULL) {  //reach the end of the list
//             lastNode = lastNode->next;
//      	}
//         lastNode->next = new;
//         return true;
//       }
//      } else {
//       	printf("%s \n\n", "failed to insert routing entry in linked list");
//     }
//      return false;
//  }



// routingtable_entry_t *routing_entry_node_new(int destinationID, int nextID)
// {
//  routingtable_entry_t *node = malloc(sizeof(routingtable_entry_t));

//     if (node == NULL) {
//     // error allocating memory for node; return error
//     return NULL;
//   } else {
//     //node->destNodeID = (int)malloc(sizeof(int));
//     if (node->destNodeID < 0) {
//       // error allocating memory for nodeID; 
//       // cleanup and return error
//       free(node);
//       return NULL;
//     } else {
//         // copy nodeID string into node->nodeID

//       node->destNodeID = destinationID;
//       node->nextNodeID = nextID;
//       node->next = NULL;
//       return node;
//     }
//   }
// }





// void set_delete(set_t *set)
// {
//   if (set != NULL) {
//     for (routingtable_entry_t *node = set->head; node != NULL; ) {
//       routingtable_entry_t *next = node->next;     // remember what comes next
//       free(node);         // free the node
//       node = next;          // and move on to next
//     }
//     free(set);    
//   }
// }


// int set_find(set_t *set, int destID)
// {
//   if (set == NULL) {
//     return -1; // bad set
//   } else if (set->head == NULL) {
//     return -1; // set is empty
//   } else if (destID < 0 ){
//     return -1; // nodeID is NULL
//   } else {
//        routingtable_entry_t *currentNode = set->head;
//         while ( currentNode != NULL ){  //iterate through all costs
//           if (currentNode->destNodeID == destID) { //If nodeID matches, return cost
//             return currentNode->nextNodeID;
//           }
//           currentNode = currentNode->next;
//         }
//         return -1; //not found
//   }

// }



// bool hashtable_insert(routingtable_t *rt, int destID, int nextID)
// {
//   if (rt != NULL && nextID > 0 && destID > 0) {
//     int hash = makehash(destID);
//     return (set_insert(rt->table[hash], destID, nextID));
//    } else {
//       return false;
//    }
   
// }


// int hashtable_find(routingtable_t *rt, int destID)
// {
//   if (rt != NULL && destID > 0) {
//     int hash = makehash(destID);
//     return set_find(rt->table[hash], destID);

//   } else {
//     return -1;
//   }

// }

//   void hashtable_delete(routingtable_t *rt)
// {
//     int i;
//     if (rt != NULL) {
//     for (i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
//       set_delete(rt->table[i]);
//    }
//       // free table of arrays
//       free(rt->table);
//       //free hashtable
//       free(rt);
//     }

// }


dv_t *dvtable_create() 
{

int nbrNum = topology_getNbrNum();

int myNodeID = topology_getMyNodeID();
int* nbrARRAY = topology_getNbrArray();
       

        dv_t* dv_table = (dv_t*)malloc((nbrNum + 1) * sizeof(dv_t)); //pointers to the headnodes
        if(!dv_table){
        	return NULL;
        }
        
        for (int i = 0; i < (nbrNum); i++) {
                dv_table[i].nodeID = nbrARRAY[i];
                dv_table[i].dvEntry = dv_entry_new((int)nbrARRAY[i]);
        }
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

	for(int i = 0; i < (nbrNum+1); i++){
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

	for(int i = 0; i < (nbrNum+1); i++){
		dv_t current_dv = dvtable[i];
		if(fromNodeID == current_dv.nodeID){
			for(int index = 0; index < netnodeNum; index++){
				if(current_dv.dvEntry[index].nodeID == toNodeID){
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

	for(int i = 0; i < (nbrNum+1); i++) {
		dv_t current_dv = dvtable[i];
		if(fromNodeID == current_dv.nodeID) {
			for(int index = 0; index < netnodeNum; index++) {
				if(current_dv.dvEntry[index].nodeID == toNodeID) {
						return current_dv.dvEntry[index].cost;
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
	for(int i = 0; i < (nbrNum+1); i++) {
		printf("~NBR Source NodeID~> :%d \n", dvtable[i].nodeID);
		for (int index = 0; index < netnodeNum; index++){
			printf(" NBR Source NodeID :%d Destination NodeID :%d Src to destination cost link: %d \n\n", dvtable[i].nodeID, dvtable[i].dvEntry[index].nodeID, dvtable[i].dvEntry[index].cost);
		}
	}
	printf("----------------------------------\n");
}


dv_entry_t *dv_entry_new(int sourceNode)
{

		int netnodeNum = topology_getNodeNum();
		int myNodeID = topology_getMyNodeID();
printf("dv_entry_new~~>netnodeNumber ~> %d\n", netnodeNum);

		int* nodeARRAY = topology_getNodeArray();

		for(int l=0; l < netnodeNum; l++){
			printf("dv_entry_new~~>nodeArray[i] %d\n", (int)nodeARRAY[l]);
		}
        
        	dv_entry_t *entries = (dv_entry_t *)malloc(netnodeNum  * sizeof(dv_entry_t));
        	if (!entries){
        		return NULL;
        	}
      
        	for(int i = 0; i < netnodeNum; i++){
        	
        		entries[i].nodeID = (int)nodeARRAY[i];
				entries[i].cost = topology_getCost(sourceNode, (int)nodeARRAY[i]);
				printf("dv_entry_new making elements~>  entries[i].nodeID: %d. entries[i].cost%d \n", entries[i].nodeID, entries[i].cost);
		}
		free(nodeARRAY);
    
    return entries;
}



int topology_getNodeIDfromname(char* hostname) {

// get host structure from gethostbyname

    struct hostent *host;
  char* ip_address;
  // if the hostname is the localhost 
  if (strcmp(hostname, "localhost") == 0){
    return topology_getMyNodeID();
 }

 // get the structure used for the network domain name 
  host = gethostbyname(hostname);
  ip_address = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

// parse 
 const char ch = '.';
 char* last;

 last = strrchr(ip_address, ch);
 
    return atoi(last + 1);
}

int* topology_getNbrArray() {
	//define variables
	char host1[256];
	char host2[256];
	char hostname[256];
	int nodeNumber = 0;
	int link_cost;
	int node_id1, node_id2, i;
	int* nodeArray;
	int nodes[2048];
	int status;
	int My_node_id;
	int nbrNum = topology_getNbrNum();
	// clear memory
	memset(&nodes, 0, (sizeof(int)*2048));
	// get domain information in format of newtork
	status = gethostname(hostname, 2048);

	// get node ID
	My_node_id = topology_getNodeIDfromname(hostname);
	
	// error checking
	if (status < 0) {
  	fprintf(stderr, "startup: unknown host name %s\n", hostname);
    return NULL;
   }
   // open topology.dat
   FILE* fp = fopen(TOPOLOGYFILE, "r");
   if (fp == NULL) {
   fprintf(stderr, "startup: unknown host name %s\n", TOPOLOGYFILE);
    return NULL;
   }
   // go through each line of file and get domain names
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {

   	node_id1 = topology_getNodeIDfromname(host1);
   	node_id2 = topology_getNodeIDfromname(host2);

   	// if domain name is the localhost name put other domain name in talbe
   	if (node_id1 == My_node_id) {
   		nodes[nodeNumber] = node_id2;
   		nodeNumber++;

   		
   	} else if (My_node_id == node_id2) {
   		nodes[nodeNumber] = node_id1; 
   		nodeNumber++;
   		
   	} 
  continue;
   	
}
// close topology.dat
fclose(fp);
	// allocate memory
	nodeArray = (int* ) malloc(nodeNumber * sizeof(int));
	if (!nodeArray) {
		fprintf(stderr, "%s\n", "Could not malloc");
	}
	// go through node array and input nodes into node array
	for (i = 0; i < nbrNum; i++) {

		if (nodes[i] > 0) {
			printf("nbrARRAY elements: %d \n", nodeArray[i]);
			nodeArray[i] = nodes[i];
		}
	}

return nodeArray;
}


int topology_getMyNodeID() {
	 char hostname[2048];
  int node_id;
  gethostname(hostname, 2048);
 
  node_id = topology_getNodeIDfromname(hostname);
  // error checking
  if(node_id < 0){
  	fprintf(stderr, "startup: unknown host name %s\n", hostname);
    exit(-1);
  }
return node_id; 
}

int topology_getNbrNum() {

	int myNodeID;
	int nbrNum; 
	// get local host node ID
	//printf("%s\n", "getmynodeID");
	myNodeID = topology_getMyNodeID();

	char host1[256];
	char host2[256];
	int link_cost;
printf("%s\n", "opened file");
	// open file topology.dat
	FILE *fp = fopen(TOPOLOGYFILE, "r");
	if (fp == NULL) {
		printf("cant open file\n");
		return -1;
	}
	//printf("%s\n", "scanning files");
	nbrNum=0;
	// go through lines of the file and collect domain names
	while (fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF ) {

			if (topology_getNodeIDfromname(host1) == myNodeID || topology_getNodeIDfromname(host2) == myNodeID) {
					nbrNum++;
			}
		}
	

// close topology.dat
printf("%s\n", "closing file");
	fclose(fp);
	
return nbrNum; 
}

unsigned int topology_getCost(int fromNodeID, int toNodeID) {

	char host1[256];
	char host2[256];
	int link_cost;
	int node_id1, node_id2;

	if(fromNodeID == toNodeID){
		return 0;
	}

	//  open topology.dat file
   FILE* fp = fopen(TOPOLOGYFILE, "r");
   if (fp == NULL) {
   fprintf(stderr, "startup: unknown host name %s\n", TOPOLOGYFILE);
    return -1;
   }
   //	go through each line of file
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {
   	 	node_id1 = topology_getNodeIDfromname(host1);
   		node_id2 = topology_getNodeIDfromname(host2);
   		// get link_cost
   		if ((fromNodeID == node_id1 && toNodeID == node_id2) || (fromNodeID == node_id2 && toNodeID == node_id1)) {
   			return link_cost;
   		}

   	}
   	fclose(fp);

return 0;
}


int topology_getNodeNum() {
	// define variables
	FILE* fp;
	char host1[256];
	char host2[256];
	int nodeNumber = 0;
	int link_cost;
	int node_id1, node_id2;
	int nodes[2048];

	// open topology.dat file
	fp = fopen(TOPOLOGYFILE, "r");
	if (fp == NULL) {
		printf("cant open file\n");
		return -1;
	}
	//clear memory
	memset(&nodes, 0, (sizeof(int)*2048));

	// go through file line by line and get domain info

	while (fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {
		node_id1 = topology_getNodeIDfromname(host1);
		if (nodes[node_id1] == 0) {
		nodes[node_id1] = node_id1;
		nodeNumber++;
		}
		node_id2 = topology_getNodeIDfromname(host2);
		if (nodes[node_id2] == 0) {
			nodes[node_id2] = node_id2;
			nodeNumber++;
		} else {
			continue;
		}
		
		
	}

// close
fclose(fp);
return nodeNumber; 
}


int* topology_getNodeArray() {

	// define variables
	char host1[256];
	char host2[256];
	char hostname[256];
	int nodeNumber = 0;
	int link_cost;
	//int node_id1, node_id2;
	int node_id, i;
	int* nodeArray;
	int nodes[2048];
	int status;
	int node_id1, node_id2;

	int My_node_id = topology_getMyNodeID();

	int NbrNum = topology_getNbrNum();
 int nodeNum =topology_getNodeNum();
	// clear memory
	memset(&nodes, 0, (sizeof(int)*2048));

	// get the domain name to network format
	status = gethostname(hostname, 256);
	// error check
	if (status < 0) {
  	fprintf(stderr, "startup: unknown host name %s\n", hostname);
    return NULL;
   }
   // open file
   FILE* fp = fopen(TOPOLOGYFILE, "r");
   if (fp == NULL) {
   	printf("cant open file\n");
    return NULL;
   }
   // go though topology.dat and get domain names
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {

   	node_id1 = topology_getNodeIDfromname(host1);
   	node_id2 = topology_getNodeIDfromname(host2);

   	// if domain name is the localhost name put other domain name in talbe
   	if (node_id1 == My_node_id) {
   		nodes[nodeNumber] = node_id2;
   		nodeNumber++;

   		
   	} else if (My_node_id == node_id2) {
   		nodes[nodeNumber] = node_id1; 
   		nodeNumber++;
   		
   	} 
  continue;
   	
}
nodes[nodeNumber] = My_node_id;
	// close file
	fclose(fp);

	// get ID name 
	//node_id = topology_getNodeIDfromname(hostname);

	// allocate memory for node Array
	nodeArray = (int* ) malloc(nodeNumber * sizeof(int));
	if (!nodeArray) {
		fprintf(stderr, "%s\n", "couldnt make array");
	}
	int index = 0;
	// go through previous node array and save nodes to nodeArray
	for (i = 0; i <=  nodeNumber; i++) {

		if (nodes[i] > 0) {
			printf("saving to nodeArray%d\n", nodes[i]);
			nodeArray[i] = nodes[i];
		}
	}
return nodeArray;
}

void topology_delete_NodeArray(int* nodeArray)
{
free(nodeArray);
}







