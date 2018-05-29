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

    // Creates Server socket
    *server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (*server_socket == -1)
    {
        perror("Socket() function failed ");
        exit(1);
    }

    // Binds Server socket to the defined port
    success = bind(*server_socket, server_info->ai_addr, server_info->ai_addrlen);
    if (success == -1)
    {
        perror("Bind() function failed ");
        close(*server_socket);
        exit(1);
    }

    // Frees struct addrinfo "server_info" structure
    freeaddrinfo(server_info);

    // Server set to listen to clients in defined port
    success = listen(*server_socket, max_clients);
    if (success == -1)
    {
        perror("Listen() function failed ");
        close(*server_socket);
        exit(1);
    }

    printf("Server waiting for Clients!\n");
}


// Getss connectec Client's IP address
void *get_client_address(struct sockaddr *client)
{
    // IPv4 IP address
    if (client->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)client)->sin_addr);
    }
    // IPv6 IP address
    else
    {
        return &(((struct sockaddr_in6 *)client)->sin6_addr);
    }
}

void accept_clients(int *server_socket, int *client_socket)
{
    // Variable declaration section
    struct sockaddr_storage client_address;
    socklen_t ip_length;
    int child;
    int success = -1;
    char ip[INET6_ADDRSTRLEN];
    int receive_size = 256;
    char buffer[receive_size];
    char file_name[receive_size];
    int file_size = 0;
    int bytes_received = -1; 

    while(1)
    {
        // Server set to accept up to 10 different clients
        ip_length = sizeof(client_address);
        *client_socket = accept(*server_socket, (struct sockaddr *)&client_address, &ip_length);
        if (*client_socket == -1)
        {
            perror("Accept() function failed");
            exit(1);
        }

        // Server gets IP address of conneceted client
        inet_ntop(client_address.ss_family, get_client_address((struct sockaddr *)&client_address), ip, sizeof(ip));
        printf("Server: Got connection from %s\n", ip);

        // Creates Sever child which will handle the different clients accepted
        child = fork();
        if (child == 0)
        {
            // Sever parent socket no longer needed
            close(*server_socket);
            
            // Receives name of file
            success = recv(*client_socket, buffer, receive_size - 1, 0);
            if (success == -1)
            {
                fprintf(stderr, "Recv() function failed");
                close(*client_socket);
                exit(1);
            }
            sprintf(file_name, "%s", "./download/");
            sprintf(file_name, "%s", buffer);
            printf("Name of file to be received: %s\n", file_name);

            // Receives the file size 
            success = recv(*client_socket, buffer, receive_size - 1, 0);
            if (success == -1)
            {
                fprintf(stderr, "Recv() function failed");
                close(*client_socket);
                exit(1);
            }
            file_size = atoi(buffer);
            printf("Size of file to be received: %d\n", file_size);

            FILE *file;
            file = fopen("test2.txt", "w");
            while (bytes_received != 0)
            {
                bytes_received = recv(*client_socket, buffer, receive_size - 1, 0);
                if (bytes_received == -1)
                {
                    fprintf(stderr, "Recv() function failed");
                    close(*client_socket);
                    fclose(file);
                    exit(1);
                }
                printf("Bytes received: %d\n", bytes_received);
                if(bytes_received != 0)
                {
                    fwrite(buffer, 1, 1, file);
                }
                bzero(buffer, receive_size);
                //printf("Bytes received: %d\n", bytes_received);
            }
            // Closes accepted client socket
            close(*client_socket);
            fclose(file);
            exit(1);
        }
        // Closes accepted client socket
        close(*client_socket);
    }
}