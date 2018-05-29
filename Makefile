.POSIX:
GCC = gcc
CFLAGS = -Wall -g -c
LINK = -o

all: myServer

myServer: server.o server_setup.o
	$(GCC) $(LINK) myServer server.o server_setup.o
server.o: server.c
	$(GCC) $(CFLAGS) server.c
server_setup.o: server_setup.h server_setup.c
	$(GCC) $(CFLAGS) server_setup.h server_setup.c
clean:
	rm -rf *.o myServer server_setup.h.gch