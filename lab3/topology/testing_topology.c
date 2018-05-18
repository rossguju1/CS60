
// Ross Guju
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>   
#include <strings.h>   
#include "../overlay/neighbortable.h"
//#include "topology.h"

int topology_getNodeIDfromname(char* hostname);

/**
 * This function returns the node ID based on an IP address. 
 *
 * Return -1 if my node ID can't be retrieved, otherwise 1
 */
int topology_getNodeIDfromip(struct in_addr* addr);

/**
 * This functions returns my node ID. 
 *
 * Return -1 if my node ID can't be retrieved, otherwise 1
 */
int topology_getMyNodeID();

/**
 * This functions parses the topology information stored in 
 * topology.dat
 *
 * Return the number of neighbors
 */
int topology_getNbrNum();

/**
 * This functions parses the topology information stored in 
 * topology.dat
 *
 * Return the number of total nodes in the overlay
 */
int topology_getNodeNum();

/**
 * This functions parses the topology information stored in 
 * topology.dat, and returns a dynamically allocated array which 
 * contains all the nodes' IDs in the overlay network. 
 *
 */
int* topology_getNodeArray();

/**
 * This functions parses the topology information stored in 
 * topology.dat and returns a dynamically allocated array which 
 * contains all the neighbors'IDs
 */
int* topology_getNbrArray();

/**
 * This functions parses the topology information stored in topology.dat
 *
 * @param fromNodeID
 * @param toNodeID
 * Return the cost of the direct link between the two given nodes. 
 * Return INFINITE_COST if no direct link between the two nodes. 
 */
unsigned int topology_getCost(int fromNodeID, int toNodeID);





int main() {
char hostname_test[] = "flume.cs.dartmouth.edu";
int getNodeID_1;

printf("%s\n", "~~~~~~~>> |TESTING topology.c| <<~~~~\n");


getNodeID_1 = topology_getNodeIDfromname(hostname_test);

printf("%s\n", "FNCT 1 ~~~> topology_getNodeIDfromname()");
printf("%s\n", "input:= char hostname_test[] = localhost");
printf("OUTPUT:= getNodeID_1 ~->%d \n", getNodeID_1);



int getNodeID_2;
struct in_addr* addr;
char flume[] = "129.170.214.115";
// 127.0.0.1 = 0x7f.1

inet_aton(flume, addr);

getNodeID_2 = topology_getNodeIDfromip(addr);


printf("OUTPUT getNodeID_2 = topology_getNodeIDfromip(addr) ~->%d \n", getNodeID_2);





int my_host_ID;


printf("%s\n", "FNCT 3 ~~~> topology_getMyNodeID()");

my_host_ID = topology_getMyNodeID();






int my_nbr_number;
my_nbr_number = topology_getNbrNum();




printf("FNCT 4 ~~~> topology_getNbrNum() ~~~> OUTPUT ~->%d \n", my_nbr_number);



int total_nbr;

total_nbr = topology_getNodeNum();

printf("FNCT 5 ~~~> topology_getNodeNum() ~~~~> OUTPUTv  ~->%d \n", total_nbr);




topology_getNodeArray();
printf("%s\n", "FNCT 6 ~~~> int* topology_getNodeArray()");



topology_getNbrArray();
printf("%s\n", "FNCT 7 ~~~> int* topology_getNbrArray()");

printf("%s\n", "topology_getNodeArray() :=> output :=> array of all Nbrs relative to local IDs in overlay (4 of them)");

topology_getCost(207, 87);


printf("in main getCost: ~~~> %d \n", topology_getCost(207, 87));

	return 0;
}

/**************** functions ****************/
// TODO: return node ID of the given hostname. 
// 1) get host structure from gethostbyname
// 2) get ip address using inet_ntoa(host->h_addr_list[0].sin_addr)
// 3) find index of last . of ip address, call it i
// 4) return atoi(ipaddress + i + 1) // + 1 to get character after .
// 4) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromname(char* hostname) {

// get host structure from gethostbyname

    //char *IPbuffer;
    struct hostent *host;

  struct in_addr address;
  char* ip_address, ip_address_2;
  if (strcmp(hostname, "localhost") == 0){
    return topology_getMyNodeID();
 }

 
     host = gethostbyname(hostname);
  ip_address = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

printf("function 2 in function one output : %d \n", ip_address_2);
const char ch = '.';
 char* ret;

 ret = strrchr(ip_address, ch);
 printf("parsed IP:%s\n", ret);
 
   
   printf("1output1~~%d\n", atoi(ret + 1));

    return atoi(ret+1);
}


// TODO: return the node ID based on an IP address.  
// 1) ipaddress = inet_ntoa(*addr)
// 2) find index of last . of ip adress, call it i
// 3) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromip(struct in_addr* addr) {
	char* ipaddress;
	ipaddress = inet_ntoa(*((struct in_addr*) addr));
	 printf(" ipaddres2: %s \n", ipaddress);


const char ch = '.';
  char* ret;

   ret = strrchr(ipaddress, ch);

   printf("pared2output2~~%d\n", atoi(ret+  1));
    return atoi(ret + 1);

}

// TODO: return my node ID. 
// Pseudocode
// 1) get system information in ugnm using uname syscall
// 2) reutnr getNodeIDfromname(ugnm.nodename)
int topology_getMyNodeID() {
	 char hostname[1024];
  int node_id;
  gethostname(hostname, 1024);
  //{
  	//fprintf(stderr, "startup: unknown host name %s\n", hostname);
    //exit(-1);
 // }
   
   printf("topology_getMyNodeID()~~~> HOSTNAME:%s\n", hostname);
  node_id = topology_getNodeIDfromname(hostname);

  //if(node_id == -1){
  	//fprintf(stderr, "startup: unknown host name %s\n", hostname);
    //exit(-1);
  //}
  printf("~~~<><><~~frunction3__getMyNodeID-> %d\n", node_id);
	return node_id; 
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

	int myNodeID;
	int nbrNum; 
	myNodeID = topology_getMyNodeID();

	char* line;
	size_t length;
	ssize_t read;
	char host1[122];
	char host2[122];
	int link_cost;

	char filename[] = "topology.dat";
	//char directory[] = "topology";

	//char *file_path = malloc(strlen(directory) + sizeof(char) + 1);
        //sprintf(file_path, "%s/%s", directory, filename);
	//try to read file given by path
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("cant open file\n");
		//free(file_path);
		return -1;
	}
	line = NULL;
	length = 0;
	nbrNum=0;
	while (fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF ) {
			printf("LINE~~>host1: %s, host2: %s, link_cost: %d\n", host1, host2, link_cost);
			printf("LINE~~>host1:NodeId: %d \n", topology_getNodeIDfromname(host1));
			printf("LINE~~>host2:NodeId: %d \n", topology_getNodeIDfromname(host2));

			if (topology_getNodeIDfromname(host1) == myNodeID || topology_getNodeIDfromname(host2) == myNodeID) {
					nbrNum++;
			}
		}
	

	fclose(fp);
	//free(file_path);
	printf("topology_getNbrNum() := nbrNum-> %d \n", nbrNum);
	return nbrNum; 
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
	FILE* fp;
	char host1[122];
	char host2[122];
	int nodeNumber = 0;
	int link_cost;
	int node_id1, node_id2;
	int nodes[10000];

	char filename[] = "topology.dat";
	//char directory[] = "topology";

	//char *file_path = malloc(strlen(directory) + sizeof(char) + 1);
    //    sprintf(file_path, "%s/%s", directory, filename);
	//try to read file given by path
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("cant open file\n");
		//free(file_path);
		return -1;
	}
	memset(&nodes, 0, (sizeof(int)*1000));

	while (fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {
		node_id1 = topology_getNodeIDfromname(host1);
		printf("~~>>>topology_getNodeIDfromname := node_id1 -> %d \n", node_id1);
		if (nodes[node_id1] == 0) {
		nodes[node_id1] = node_id1;
		nodeNumber++;
		}
		node_id2 = topology_getNodeIDfromname(host2);
		printf("~~>>>topology_getNodeIDfromname := node_id2 -> %d \n", node_id2);
		if (nodes[node_id2] == 0) {
			nodes[node_id2] = node_id2;
			nodeNumber++;
		} else {
			continue;
		}
		
		
	}

	nodeNumber = nodeNumber - 1;
	fclose(fp);
	//free(file_path);
		printf("~~topology_getNodeNum := nodeNumber-> %d \n", nodeNumber);

	return nodeNumber; //nodeNumber-1;
}


// TODO: parse the topology file and return a dynamically allocated 
// array with all nodes' IDs in the overlay network. 
// 1) Same steps to parse file above
// 2) Allocate nodeArray
// 3) Copy nodes[] into nodeArray[]
// 4) Return nodeArray[]
int* topology_getNodeArray() {


	char host1[122];
	char host2[122];
	char hostname[1000];
	int nodeNumber = 0;
	int link_cost;
	//int node_id1, node_id2;
	int node_id, i;
	int* nodeArray;
	int nodes[1000];
	int status;
	char filename[] = "topology.dat";


	memset(&nodes, 0, (sizeof(int)*1000));


	status = gethostname(hostname, 1000);
	printf("localhost~~>address~~> %s\n", hostname);

	//if (status < 0) {
  	//fprintf(stderr, "startup: unknown host name %s\n", hostname);
    //return NULL;
   //}

   FILE* fp = fopen(filename, "r");
   if (fp == NULL) {
   	printf("cant open file\n");
   //fprintf(stderr, "startup: unknown host name %s\n", filename);
    return NULL;
   }
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {

   	node_id = topology_getNodeIDfromname(host1);
   	printf("node_id1~~> %d \n", node_id);
   	(nodes[node_id] == 0) ? nodes[node_id] = node_id : nodeNumber++;
   	node_id = topology_getNodeIDfromname(host2);
   	printf("node_id2~~> %d \n", node_id);
   	(nodes[node_id] == 0) ? nodes[node_id] = node_id : nodeNumber++;
}
	fclose(fp);

	printf("localhost~~>address~~> %s\n", hostname);	

	node_id = topology_getNodeIDfromname(hostname);

	printf("localhost~~>NODE__-_-~~>Node_Id %d\n", node_id);	

	printf("%s\n", "<~~dynamically alloc mem for nodeArray~~>>\n");

	nodeArray = (int* ) malloc(nodeNumber * sizeof(int));
	//if (!nodeArray) {
		//fprintf(stderr, "%s\n", "ERRRRRRRRor");
	//}
	int index = 0;

	for (i = 0; i < 1000; i++) {

		if (nodes[i] >0) {
		printf("Node Id ~> %d is located in index ~~>%d in nodeArray[]\n", i, index);

			nodeArray[index++] = i;
		}
	}
	return nodeArray;
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

char host1[122];
	char host2[122];
	char hostname[1000];
	int nodeNumber = 0;
	int link_cost;
	//int node_id1, node_id2;
	int node_id1, node_id2, i;
	int* nodeArray;
	int nodes[1000];
	int status;
	int My_node_id;
	char filename[] = "topology.dat";


	memset(&nodes, 0, (sizeof(int)*1000));


	status = gethostname(hostname, 1000);
	printf("localhost~~>address~~> %s\n", hostname);

	My_node_id = topology_getNodeIDfromname(hostname);
	printf("localhost~~>Node_Id~> %d\n", My_node_id);
	

	if (status < 0) {
  	fprintf(stderr, "startup: unknown host name %s\n", hostname);
    return NULL;
   }

   FILE* fp = fopen(filename, "r");
   if (fp == NULL) {
   fprintf(stderr, "startup: unknown host name %s\n", filename);
    return NULL;
   }
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {

   	node_id1 = topology_getNodeIDfromname(host1);
   	node_id2 = topology_getNodeIDfromname(host2);
   	printf("node_id1~~> %d \n", node_id1);
   	printf("node_id2~~> %d \n", node_id2);

   	if (node_id1 == My_node_id) {
   		nodes[nodeNumber] = node_id2;
   		nodeNumber++;

   		
   	} else if (My_node_id == node_id2) {
   		nodes[nodeNumber] = node_id1; 
   		nodeNumber++;
   		
   	} 
   	continue;
   	
}
fclose(fp);

		nodeArray = (int* ) malloc(nodeNumber * sizeof(int));
	if (!nodeArray) {
		fprintf(stderr, "%s\n", "ERRRRRRRRor");
	}
	//int index = 0;

	for (i = 0; i < nodeNumber; i++) {

		if (nodes[i] > 0) {
		printf("Node Id ~> %d is located in index ~~>%d in nodes[]\n", nodes[i], i);

			nodeArray[i] = nodes[i];

		 printf("Node Id ~> %d is located in index ~~>%d in nodeArray[]\n", nodeArray[i], i);

		}
	}



	return nodeArray;
}


// TODO: parse the topology information stored in the topology file 
// and return the link cost between two given nodes. 
unsigned int topology_getCost(int fromNodeID, int toNodeID) {

char host1[122];
	char host2[122];
	//int nodeNumber = 0;
	int link_cost;
	//int node_id1, node_id2;
	int node_id1, node_id2;

	//int status;
	//int My_node_id;
	char filename[] = "topology.dat";


   FILE* fp = fopen(filename, "r");
   if (fp == NULL) {
   //fprintf(stderr, "startup: unknown host name %s\n", filename);
    return -1;
   }
   while(fscanf(fp, "%s %s %d", host1, host2, &link_cost) != EOF) {
   	 	node_id1 = topology_getNodeIDfromname(host1);
   	 	printf("host1 ID in topology.dat file is %d AND fromNodeID is %d\n", node_id1, fromNodeID);
   		node_id2 = topology_getNodeIDfromname(host2);
   		printf("host2 ID in topology.dat file is %d AND fromNodeID is %d\n", node_id2, toNodeID);

   		if (fromNodeID == node_id1 && toNodeID == node_id2) {
   			printf("In topology.dat, link_cost from %d to %d is := %d\n", fromNodeID, toNodeID, link_cost);
   			return link_cost;
   		}

   	}

	return 0;
}
