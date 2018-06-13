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
void *get_server_address(struct sockaddr *);

// Allows Client to choose file and path
void choose_file(char *, char *, int);

// Allows client to send Server commands 
void set_path(int *);

// Sends file to Server
void send_file_to_server(int *);

// Recevies file from Server
void receive_file_from_server(int *);





#endif // _CLIENT_SETUP_H
