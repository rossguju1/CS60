# Makefile for CS60 Lab1
#
# CS60
# Ross Guju


all: TCPClient TCPServer_v1 TCPServer_v2 TCPServer_v3 TCPServer_v4 

TCPClient:
	gcc -Wall -pedantic -std=c11 -w -ggdb -o TCPClient TCPClient.c

TCPServer_v1:
	gcc -Wall -pedantic -std=c11 -ggdb -o TCPServer_v1 TCPServer_v1.c

TCPServer_v2:
	gcc -Wall -pedantic -std=c11 -ggdb -o TCPServer_v2 TCPServer_v2.c

TCPServer_v3:
	gcc -lpthread -Wall -pedantic -std=c11 -ggdb -o TCPServer_v3 TCPServer_v3.c

TCPServer_v4:
	gcc -Wall -pedantic -std=c11 -ggdb -o TCPServer_v4 TCPServer_v4.c

clean:
	rm -rd *.dSYM

