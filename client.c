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
    struct addrinfo temp_info, *client_info;

    ipv = 3;

    // Obtains and sets information regaring Client 
    set_client_info(&temp_info, &client_info, ipv, argv[1], PORT);

    // Sets up Client and connects to desired Server
    setup_client(client_info, &client_socket);

	return 0;
}
