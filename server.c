#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "server_setup.h"

#define PORTNUMBER "9440"
#define MAXCLIENTS 10

int main(int argc, char *argv[])
{
    // Variable Declaration Section
    int server_socket;
    int client_socket;
    struct addrinfo temp_info, *server_info;
    
    // Obtains and sets information regarding Server 
    set_server_info(&temp_info, &server_info, 3, 1, PORTNUMBER);

    // Sets up server
    setup_server(server_info, &server_socket, MAXCLIENTS);

    // Sever starts and accepts up to 10 clients at once
    accept_clients(&server_socket, &client_socket);

    return 0;
}