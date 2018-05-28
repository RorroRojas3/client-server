#ifndef _SERVER_SETUP_
#define _SERVER_SETUP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void set_server_info(struct addrinfo *, struct addrinfo **, int, int, char *);
void setup_server(struct addrinfo *, int *, int);
void accept_clients(int *, int *);


#endif // _SERVER_SETUP_