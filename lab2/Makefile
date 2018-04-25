## Ross Guju
#CS 60

all: simple stress

simple: client/app_simple_client.o server/app_simple_server.o client/mrt_client.o server/mrt_server.o common/seg.o
	gcc -Wall -pthread -pedantic -std=c11 server/app_simple_server.o common/seg.o server/mrt_server.o -o server/simple_server
	gcc -Wall -pthread -pedantic -std=c11 client/app_simple_client.o common/seg.o client/mrt_client.o -o client/simple_client

stress: client/app_stress_client.o server/app_stress_server.o client/mrt_client.o server/mrt_server.o common/seg.o
	gcc -Wall -pthread -pedantic -std=c11 -ggdb server/app_stress_server.o common/seg.o server/mrt_server.o -o server/stress_server
	gcc -Wall -pthread -pedantic -std=c11 -ggdb client/app_stress_client.o common/seg.o client/mrt_client.o -o client/stress_client

client/app_simple_client.o: client/app_simple_client.c 
	gcc -Wall -pthread -pedantic -std=c11 -ggdb -c client/app_simple_client.c -o client/app_simple_client.o 
server/app_simple_server.o: server/app_simple_server.c 
	gcc -Wall -pthread -pedantic -std=c11 -ggdb -c server/app_simple_server.c -o server/app_simple_server.o

client/app_stress_client.o: client/app_stress_client.c 
	gcc -Wall -pthread -pedantic -std=c11 -ggdb -c client/app_stress_client.c -o client/app_stress_client.o 
server/app_stress_server.o: server/app_stress_server.c 
	gcc -Wall -pthread -pedantic -std=c11 -ggdb -c server/app_stress_server.c -o server/app_stress_server.o

common/seg.o: common/seg.c common/seg.h
	gcc -Wall -pedantic -std=c11 -ggdb -c common/seg.c -o common/seg.o
client/mrt_client.o: client/mrt_client.c client/mrt_client.h 
	gcc -pthread -w -Wall -pedantic -std=c11 -ggdb -g -c client/mrt_client.c -o client/mrt_client.o
server/mrt_server.o: server/mrt_server.c server/mrt_server.h
	gcc -pthread -Wall -w -pedantic -std=c11 -ggdb -g -c server/mrt_server.c -o server/mrt_server.o

clean:
	rm -rf client/*.o
	rm -rf server/*.o
	rm -rf common/*.o
	rm -rf client/simple_client
	rm -rf server/simple_server
	rm -rf client/stress_client
	rm -rf server/stress_server

