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

// Displays the restricted directories
void set_path(char *, int *, int, char *);

// Receives files from Client
void receive_file(int *, int);

// Deletes file based on Client input
void delete_file(int *, int);

// Sends a file to Client
void send_file_to_client(int *, int);

// Gets connectec Client's IP address
void *get_client_address(struct sockaddr *);

// Sever starts and accepts up to 10 clients at once
void accept_clients(int *, int *);









#endif // _SERVER_SETUP_
