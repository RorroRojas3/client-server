#ifndef _SERVER_SETUP_
#define _SERVER_SETUP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

// Obtains and sets information regaring Server 
void set_server_info(struct addrinfo *, struct addrinfo **, int, int, char *);

// Sets up server
void setup_server(struct addrinfo *, int *, int);

// Sever starts and accepts up to 10 clients at once
void accept_clients(int *, int *);

// Gets connectec Client's IP address
void *get_client_address(struct sockaddr *);

// Displays the restricted directories
void display_directories(char *, int *);

// Deletes files/directory
void delete_file(int *, char *);

#endif // _SERVER_SETUP_
