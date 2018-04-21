# Mini Reliable Transport

Use the provided Makefile to build the code. 

There are two executables, a simple executable and a stress executable for you to test with.
* `make`: make both the simple and stress executables.  
* `make simple`: build just the simple version. 
* `make stress`: build just the stress version. 

Use ./simple_client to run the simple client and ./simple_server to run the simple server.  
The stress versions can be run in the same way with simple replaced with stress.

### Top directory:
send_this_text.txt - simple test data

### File breakdown:
##### client directory:
* app_simple_client.c - simple client application
* app_stress_client.c - stress client application
* mrt_client.c - MRT client side source file
* mrt_client.h - MRT client side header file

##### server directory:
* app_simple_server.c - simple server application
* app_stress_server.c - stress server application
* mrt_server.c - MRT server source file
* mrt_server.h - MRT server header file

##### common directory:
* seg.c - MNP function implementation, act on segments
* seg.h - MNP function header
* constants.h - define some useful constants

**Your job is to implement the functions in the various .c files  that have been left empty (i.e. they just return immediately).**
