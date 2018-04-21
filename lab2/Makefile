# Makefile for Mini Reliable Transport
#
# CS 60, March 2018

MAKE = make

############## default: make all libs and programs ##########
all: 
	$(MAKE) -C client
	$(MAKE) -C server

simple: 
	$(MAKE) -C client simple-client
	$(MAKE) -C server simple-server

stress: 
	$(MAKE) -C client stress-client
	$(MAKE) -C server stress-server

.PHONY: valgrind clean

############## valgrind all programs ##########
valgrind: all
	$(MAKE) -C client valgrind
	$(MAKE) -C server valgrind

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM
	$(MAKE) -C client clean
	$(MAKE) -C server clean	
	$(MAKE) -C common clean	
