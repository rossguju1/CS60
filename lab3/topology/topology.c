/*
 * topology/topology.c: implementation of functions to parse the 
 * topology file 
 *
 * CS60, March 2018. 
 */

#include "topology.h"

/**************** constants ****************/
#define TOPOLOGYFILE "../topology/topology.dat"


/**************** functions ****************/
// TODO: return node ID of the given hostname. 
// 1) get host structure from gethostbyname
// 2) get ip address using inet_ntoa(host->h_addr_list[0].sin_addr)
// 3) find index of last . of ip address, call it i
// 4) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromname(char* hostname) {

	return 0;
}


// TODO: return the node ID based on an IP address.  
// 1) ipaddress = inet_ntoa(*addr)
// 2) find index of last . of ip adress, call it i
// 3) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromip(struct in_addr* addr) {
	
	return 0;
}


// TODO: return my node ID. 
// Pseudocode
// 1) get system information in ugnm using uname syscall
// 2) reutnr getNodeIDfromname(ugnm.nodename)
int topology_getMyNodeID() {

	return 0; 
}


// TODO: parse the topology file and return the number of neighbors.
// 1) myNodeID = getMyNodeID(); nbrNum = 0
// 2) open TOPOLOGYFILE
// 3) Go through each line, get host1 and host2 names
//      if host1 == myNodeID || host2 == myNodeID
//        nbrNum++
// 4) close TOPOLGYFILE
// 5) return nbrNum
int topology_getNbrNum() {

	return 0; 
}


// TODO: parse the topology file and return the number of nodes in 
// the overlay network. 
// 1) open TOPOLOGY FILE
// 2) Go through each line and get host1 and host2 names
//      node1 = getNodeIDfromname(host1)
//      for each node
//        see if node1 needs to be added
//      if node1 needs to be added
//        append node1 to node array
//      node2 = getNodeIDfromname(host2)
//      for each node
//        see if node2 needs to be added
//      if node1 needs to be added
//        append node2 to node array
// 3) close TOPOLOGYFILE
int topology_getNodeNum() {

	return 0;
}


// TODO: parse the topology file and return a dynamically allocated 
// array with all nodes' IDs in the overlay network. 
// 1) Same steps to parse file above
// 2) Allocate nodeArray
// 3) Copy nodes[] into nodeArray[]
// 4) Return nodeArray[]
int* topology_getNodeArray() {

	return 0;
}


// TODO: parse the topology file and return a dynamically allocated 
// array with all neighbors' IDs. 
// 1) open TOPOLOGYFILE
// 2) for each line of the file, get host1 and host2 names
//      node1 = getNodeIDfromname(host1)
//      node2 = getNodeIDfromname(host2)
//      if(node1 == myNodeID)
//        nodes[nodeNum] = node2; nodeNum++
//      else if (node2 == myNodeID)
//        nodes[nodeNum] = node1; nodeNum++
// 3) allocate nodeArray
// 4) copy nodes[] into nodeArray[]
int* topology_getNbrArray() {

	return 0;
}


// TODO: parse the topology information stored in the topology file 
// and return the link cost between two given nodes. 
unsigned int topology_getCost(int fromNodeID, int toNodeID) {

	return INFINITE_COST;
}
