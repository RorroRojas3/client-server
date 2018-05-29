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

void set_client_info(struct addrinfo *temp_info, struct addrinfo **server_info, int ipv, int socket_type, char *server_address, char *port_number)
{
    // Variable Declaration Section
    int success = -1;

    // Clears all information from "struct addrinfo" 
    memset(temp_info, 0, sizeof(*temp_info));
    
    // Sets IPv# based on user choice, 1 for Ipv4, 2 for IPv6, and 3 for unespecified IPv#
    if (ipv == 1)
    {
        temp_info->ai_family = AF_INET;
    }
    else if (ipv == 2)
    {
        temp_info->ai_family = AF_INET6;
    }
    else if (ipv == 3)
    {
        temp_info->ai_family = AF_UNSPEC;
    }
    else
    {
        printf("Incorrect IPv# input\n.");
        exit(1);
    }

    // Sets the type of socket, 1 for TCP, 2 for UDP
    if (socket_type == 1)
    {
        temp_info->ai_socktype = SOCK_STREAM;
    }
    else if (socket_type == 2)
    {
        temp_info->ai_socktype = SOCK_DGRAM;
    }
    else
    {
        printf("Incorrect Socket type input\n");
        exit(1);
    }

    // Sets the flag to accept either Ipv4 or Ipv6 connections
    temp_info->ai_flags = AI_PASSIVE;

    // Sets server_info to point to temp_info
    success = getaddrinfo(server_address, port_number, temp_info, server_info);
    if (success != 0)
    {
        fprintf(stderr, "Error on getaddrinfo: %s\n", gai_strerror(success));
        exit(1);
    }
}

// Getss connectec Server's IP address
void *get_server_address(struct sockaddr *server)
{
    // IPv4 IP address
    if (server->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)server)->sin_addr);
    }
    // IPv6 IP address
    else
    {
        return &(((struct sockaddr_in6 *)server)->sin6_addr);
    }
}

// Sets up Client
void setup_client(struct addrinfo *client_info, int *client_socket)
{
    // Variable declaration section
    int success;
    char ip[INET6_ADDRSTRLEN];

    // Creates Client socket
    *client_socket = socket(client_info->ai_family, client_info->ai_socktype, client_info->ai_protocol);
    if (*client_socket == -1)
    {
        perror("Socket() function failed");
        exit(1);
    }

    // Client attempts to connect to Server
    success = connect(*client_socket, client_info->ai_addr, client_info->ai_addrlen);
    if (success == -1)
    {
        perror("Connect() function failed");
        close(*client_socket);
        exit(1);
    }

    // Converts IPv4/Ipv6 addresses from binary to text form 
	inet_ntop(client_info->ai_family, get_server_address((struct sockaddr *)client_info->ai_addr), ip, sizeof(ip));
	printf("Client connected to IP:  %s\n", ip);

    // Frees struct addrinfo "client_info"
    freeaddrinfo(client_info);
}