/*
 * topology/topology.c: implementation of functions to parse the 
 * topology file 
 *
 *  ROSS GUJU
 *
 *
 * CS60, March 2018. 
 */
 #include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>   
#include <strings.h>   

#include "topology.h"

/**************** constants ****************/
#define TOPOLOGYFILE "../topology/topology.dat"


/**************** functions ****************/
// topology_getNodeIDfromname()
// input hostname : domain name
//  output int ID of domain name
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




// topology_getNodeIDfromip()
// 
// input : hostname in  struct in_addr form (domain name)
// output : return the node ID based on an IP address.  

int topology_getNodeIDfromip(struct in_addr* addr) {
	char* ipaddress;
	ipaddress = inet_ntoa(*((struct in_addr*) addr));


  const char ch = '.';
  char* last;

  last = strrchr(ipaddress, ch);

return atoi(last + 1);
}

// topology_getMyNodeID()
// input: none
// output  return my node ID. 

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

//topology_getNbrNum()
//input : NONE
// output : neighbor number
// parse the topology file and return the number of neighbors.

int topology_getNbrNum() {

	int myNodeID;
	int nbrNum; 
	// get local host node ID
	myNodeID = topology_getMyNodeID();

	char host1[256];
	char host2[256];
	int link_cost;

	// open file topology.dat
	FILE *fp = fopen(TOPOLOGYFILE, "r");
	if (fp == NULL) {
		printf("cant open file\n");
		return -1;
	}
	
	nbrNum=0;
	// go through lines of the file and collect domain names
	while (fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF ) {

			if (topology_getNodeIDfromname(host1) == myNodeID || topology_getNodeIDfromname(host2) == myNodeID) {
					nbrNum++;
			}
		}
	
// close topology.dat
	fclose(fp);
	
return nbrNum; 
}


// topology_getNodeNum()
// input : none
// output : node numbers in topology
// parse the topology file and return the number of nodes in 
// the overlay network. 

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

nodeNumber = nodeNumber - 1;
// close
fclose(fp);
return nodeNumber; 
}


// topology_getNodeArray() 
// intput : none
// output parse: the topology file and return a dynamically allocated 

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

   	node_id = topology_getNodeIDfromname(host1);
   	(nodes[node_id] == 0) ? nodes[node_id] = node_id : nodeNumber++;
   	node_id = topology_getNodeIDfromname(host2);
   	(nodes[node_id] == 0) ? nodes[node_id] = node_id : nodeNumber++;
}
	// close file
	fclose(fp);

	// get ID name 
	node_id = topology_getNodeIDfromname(hostname);

	// allocate memory for node Array
	nodeArray = (int* ) malloc(nodeNumber * sizeof(int));
	if (!nodeArray) {
		fprintf(stderr, "%s\n", "couldnt make array");
	}
	int index = 0;
	// go through previous node array and save nodes to nodeArray
	for (i = 0; i < 4; i++) {

		if (nodes[i] >0) {
			nodeArray[index++] = i;
		}
	}
return nodeArray;
}

//topology_getNbrArray()
//input : none
//output : parse the topology file and return a dynamically allocated 

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
	for (i = 0; i < nodeNumber; i++) {

		if (nodes[i] > 0) {

			nodeArray[i] = nodes[i];
		}
	}

return nodeArray;
}

// topology_getCost()
// input : none
//return : parse the topology information stored in the topology file 
// and return the link cost between two given nodes. 
unsigned int topology_getCost(int fromNodeID, int toNodeID) {

	char host1[256];
	char host2[256];
	int link_cost;
	int node_id1, node_id2;



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
   		if (fromNodeID == node_id1 && toNodeID == node_id2) {
   			return link_cost;
   		}

   	}

return 0;
}
