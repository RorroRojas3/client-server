#ifndef _CLIENT_SETUP_H
#define _CLIENT_SETUP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Obtains and sets Client's information
void set_client_info(struct addrinfo *, struct addrinfo **, int, int, char *, char *);

// Sets up Client
void setup_client(struct addrinfo *,  int *);

// Gets the IP address of Server connected
void get_server_address(struct addrinfo *);

void send_file(int *, char *);

#endif // _CLIENT_SETUP_H