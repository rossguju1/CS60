# Makefile for the overlay 
# Ross Guju
# CS 60, March 2018

C = ../common

PROGS = testing_topology
LIBS = 

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

test_topology: test_topology.o ../overlay/neighbortable.o topology.o $(C)/pkt.o 
	$(CC) $(CFLAGS) $^ -o $@

##### source dependencies
test_topology.o: ../overlay/neighbortable.h $(C)/constants.h $(C)/pkt.h topology.h 

../overlay/neighbortable.o: neighbortable.h ../topology/topology.h 

topology.o: ../topology/topology.h $(C)/constants.h

$(C)/pkt.o: $(C)/pkt.h $(C)/constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: test_topology
	$(VALGRIND) ./overlay

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)


