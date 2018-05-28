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

void set_server_info(struct addrinfo *temp_info, struct addrinfo **server_info, int ipv, int type, char *port_number)
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
    if (type == 1)
    {
        temp_info->ai_socktype = SOCK_STREAM;
    }
    else if (type == 2)
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
    success = getaddrinfo(NULL, port_number, temp_info, server_info);
    if (success != 0)
    {
        fprintf(stderr, "Error on getaddrinfo: %s\n", gai_strerror(success));
        exit(1);
    }
}

void setup_server(struct addrinfo *server_info, int *server_socket, int max_clients)
{
    // Variable Declaration Section
    int success = -1;
    *server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (*server_socket == -1)
    {
        perror("Socket() function failed ");
        exit(1);
    }

    success = bind(*server_socket, server_info->ai_addr, server_info->ai_addrlen);
    if (success == -1)
    {
        perror("Bind() function failed ");
        close(*server_socket);
        exit(1);
    }

    freeaddrinfo(server_info);

    success = listen(*server_socket, max_clients);
    if (success == -1)
    {
        perror("Listen() functio failed ");
        close(*server_socket);
        exit(1);
    }

    printf("Server waiting for Clients!\n");
}

void accept_clients(int *server_socket, int *client_socket)
{
    // Variable declaration section
    socklen_t client_address;
    struct sockaddr_storage client_address_size;
    int child;
    int success = -1;
    char message[256];
    strcpy(message, "Hello world");

    while(1)
    {
        client_address = sizeof(client_address_size);

        *client_socket = accept(*server_socket, (struct sockaddr *)&client_address_size, &client_address);
        if (*client_socket == -1)
        {
            perror("Accept() function failed");
            exit(1);
        }

        child = fork();
        if (child == 0)
        {
            close(*server_socket);
            success = send(*client_socket, message, 255, 0);
            if (success == -1)
            {
                perror("Send() function failed");
                close(*client_socket);
                exit(1);
            }
            close(*client_socket);
            exit(1);
        }
        close(*client_socket);
    }
}