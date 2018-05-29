.POSIX:
GCC = gcc
CFLAGS = -Wall -g -c
LINK = -o

all: myServer myClient

myServer: server.o server_setup.o
	$(GCC) $(LINK) myServer server.o server_setup.o
server.o: server.c
	$(GCC) $(CFLAGS) server.c
server_setup.o: server_setup.h server_setup.c
	$(GCC) $(CFLAGS) server_setup.h server_setup.c

myClient: client.o client_setup.o
	$(GCC) $(LINK) myClient client.o client_setup.o
client.o: client.c
	$(GCC) $(CFLAGS) client.c
client_setup.o: client_setup.h client_setup.c
	$(GCC) $(CFLAGS) client_setup.h client_setup.c
	
clean:
	rm -rf *.o myServer myClient server_setup.h.gch client_setup.h.gch
	
