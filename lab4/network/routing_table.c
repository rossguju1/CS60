
/*
 * network/routing_table.c: implementation of the routing table used
 * by the routing protocol of MNP. A routing table is a hash table
 * containing MAX_RT_ENTRY slot entries.
 *
 *  Ross Guju
 *
 * CS60, March 2018.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../common/constants.h"
#include "../topology/topology.h"
#include "routing_table.h"



/********** local functions ***********/

set_t *set_new(void);

routingtable_entry_t *routing_entry_node_new(int destinationID, int nextID);

bool set_insert(set_t *set, int destinationID, int nextID);

void set_delete(set_t *set);

bool hashtable_insert(routingtable_t *rt, int nodeID, int nextID);

void hashtable_delete(routingtable_t *rt);

int hashtable_find(routingtable_t *rt, int destID);

int set_find(set_t *set, int destID);

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
int makehash(int node)
{
  return node % MAX_ROUTINGTABLE_SLOTS;
}

/** TODO:
 * This function creates a routing table dynamically.
 * All the entries in the table are initialized to NULL pointers.
 * Then for all the neighbors with direct links, create a routing
 * entry with the neighbor itself as the next hop node, and insert
 * this routing entry into the routing table.
 *
 * Return The dynamically created routing table structure.
 */
routingtable_t *routingtable_create()
{


  int i; // counter for loop
  routingtable_t *rt = malloc(sizeof(routingtable_t)); // actual hashtable
  if (!rt) {
    return NULL;
  }

  rt->table = calloc(MAX_ROUTINGTABLE_SLOTS, sizeof(set_t)); //pointers to the headnodes


  printf("%s\n", "routingtable_Create 1");
  for (i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
    //create an empty set for all indexes in table
    printf("%s\n", "making a new set");
    rt->table[i] = set_new();
  }

  int myNodeID = topology_getMyNodeID();

  int nbrNum = topology_getNbrNum();
  int* nbrARRAY = topology_getNbrArray();


  for (int i = 0; i < nbrNum; i++) {
    printf("%s\n", "inserting");
    if (hashtable_insert(rt, nbrARRAY[i], nbrARRAY[i])) {
      printf("%s\n", "inserted");
      continue;
    }
  }
  return rt;
}

/** TODO:
 * This function destroys a routing table.
 * All dynamically allocated data structures for this routing table are freed.
 */
void routingtable_destroy(routingtable_t *routing_table)
{

  hashtable_delete(routing_table);

}

/** TODO:
 * This function updates the routing table using the given
 * destination node ID and next hop's node ID.
 * If the routing entry for the given destination already exists,
 * update the existing routing entry.
 * If the routing entry of the given destination is not there, add
 * one with the given next node ID.
 * Each slot in the routing table contains a linked list of routing
 * entries due to conflicting hash nodeIDs (different hash nodeIDs -
 * destination node ID - may have the same hash value (slot entry
 * number).
 * To add an routing entry to the hash table:
 * First use the hash function makehash() to get the slot number in
 * which this routing entry should be stored.
 * Then append the routing entry to the linked list in that slot.
 */
void routingtable_setnextnode(routingtable_t *routing_table, int destNodeID, int nextNodeID)
{
  if (!(hashtable_insert(routing_table, destNodeID, nextNodeID))) {
    printf(" failed to insert routing entry in table\n");
  }


}

/** TODO:
 * This function looks up the destNodeID in the routing table.
 * Since routing table is a hash table, this opeartion has O(1) time complexity.
 * To find a routing entry for a destination node, first use the
 * hash function makehash() to get the slot number and then go
 * through the linked list in that slot to search for the routing
 * entry.
 * Return nextNodeID if the destNodeID is found, else -1.
 */
int routingtable_getnextnode(routingtable_t *routing_table, int destNodeID)
{
  int nextHop = hashtable_find(routing_table, destNodeID);

  if (nextHop >= 0) {
    return nextHop;
  }

  return -1;
}

/** TODO:
 * This function prints out the contents of the routing table.
 */
void routingtable_print(routingtable_t *rt) {
  printf("-------------routing table------------\n");

  for (int i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
    printf("ROUTING TABLE SLOT number: %d \n\n", i);
    if (rt->table[i] != NULL) {
      routingtable_entry_t *currentNode = rt->table[i]->head;
      while ( currentNode != NULL ) { //iterate through all costs
        printf("make sure that this set has same destination ID: %d ~> Next Node ID %d", currentNode->destNodeID, currentNode->nextNodeID);
        currentNode = currentNode->next;
      }
    }
  }
  printf("--------------------------------------\n");
}


/*********************** set_new() ***********************/

// helper function

set_t *set_new(void)
{
  set_t *set = malloc(sizeof(set_t)); // allocate memory to *set pointer

  if ( set == NULL ) {
    return NULL;       //error allocating memory
  } else {
    set->head = NULL;  // set first node head to be NULL
    return set;
  }
}


/********************** set_insert() ***************************/

// helper function


bool set_insert(set_t *set, int destinationID, int nextID) {
  if (set != NULL && nextID > 0 && destinationID > 0) {
    // allocate a new node to be added to the list
    routingtable_entry_t *new = routing_entry_node_new(destinationID, nextID);
    if (new == NULL) {
      return false; //return false if new node is not created
    }

    if (set->head == NULL) {  //If set has no costs
      set->head = new;  //Make the new Node the head of the set
      return true;
    } else {
      routingtable_entry_t *lastNode = set->head;
      new->next = NULL;
      //compare nodeID for head
      while (lastNode->next != NULL) {  //reach the end of the list
        lastNode = lastNode->next;
      }
      lastNode->next = new;
      return true;
    }
  } else {
    printf("%s \n\n", "failed to insert routing entry in linked list");
  }
  return false;
}


/******************** routing_entry_node_new() *********************/

// helper function

routingtable_entry_t *routing_entry_node_new(int destinationID, int nextID)
{
  routingtable_entry_t *node = malloc(sizeof(routingtable_entry_t));

  if (node == NULL) {
    // error allocating memory for node; return error
    return NULL;
  } else {
    //node->destNodeID = (int)malloc(sizeof(int));
    if (node->destNodeID < 0) {
      // error allocating memory for nodeID;
      // cleanup and return error
      free(node);
      return NULL;
    } else {
      // copy nodeID values into node->nodeID

      node->destNodeID = destinationID;
      node->nextNodeID = nextID;
      node->next = NULL;
      return node;
    }
  }
}



/***********  set_delete() ******/

// helper function

void set_delete(set_t *set)
{
  if (set != NULL) {
    for (routingtable_entry_t *node = set->head; node != NULL; ) {
      routingtable_entry_t *next = node->next;     // remember what comes next
      free(node);         // free the node
      node = next;          // and move on to next
    }
    free(set);
  }
}


/************** set_find() ***************/

// helper function

int set_find(set_t *set, int destID)
{
  if (set == NULL) {
    return -1; // bad set
  } else if (set->head == NULL) {
    return -1; // set is empty
  } else if (destID < 0 ) {
    return -1; // nodeID is negative
  } else {
    routingtable_entry_t *currentNode = set->head;
    while ( currentNode != NULL ) { //iterate through all costs
      if (currentNode->destNodeID == destID) { //If nodeID matches, return cost
        return currentNode->nextNodeID;
      }
      currentNode = currentNode->next;
    }
    return -1; //not found
  }

}

/******* hashtable_insert() ************/
// helper function

bool hashtable_insert(routingtable_t *rt, int destID, int nextID)
{
  if (rt != NULL && nextID > 0 && destID > 0) {
    int hash = makehash(destID);
    return (set_insert(rt->table[hash], destID, nextID));
  } else {
    return false;
  }

}

/********** hashtable_find() ****************/

// helper function

int hashtable_find(routingtable_t *rt, int destID)
{
  if (rt != NULL && destID > 0) {
    int hash = makehash(destID);
    return set_find(rt->table[hash], destID);

  } else {
    return -1;
  }

}

/************** hashtable_delete() ****************/

// helper function

void hashtable_delete(routingtable_t *rt)
{
  int i;
  if (rt != NULL) {
    for (i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) {
      set_delete(rt->table[i]);
    }
    // free table of arrays
    free(rt->table);
    //free hashtable
    free(rt);
  }

}







