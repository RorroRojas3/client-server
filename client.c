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
#include "client_setup.h"

#define PORT "9440"

int main(int argc, char *argv[])
{
    // Variable Declaration Section
    int client_socket;
    int ipv;
    int socket_type;
    struct addrinfo temp_info, *client_info;
    
    // Checks that user has entered IP address of Server
    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./program hostname(IP) filename");
        exit(1);
    }

    ipv = 3;
    socket_type = 1;

    // Obtains and sets information regaring Client 
    set_client_info(&temp_info, &client_info, ipv, socket_type, argv[1], PORT);

    // Sets up Client and connects to desired Server
    setup_client(client_info, &client_socket);

    // Sends file to server
    send_file(&client_socket, argv[2]);

}