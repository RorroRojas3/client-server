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

void set_directory(int *client_socket)
{
    int received_bytes = 1;
    int sent_bytes = -1;
    char buffer[1024];
    char message[1024];

    while(1)
    {
        while(1)
        {
            received_bytes = recv(*client_socket, buffer, 1023, 0);
            if (received_bytes == -1)
            {
                fprintf(stderr, "Recv() function failed\n");
                close(*client_socket);
                exit(1);
            }
            else if(received_bytes > 0)
            {
                strcpy(message, buffer);
                if(strcmp(message, "Done") == 0)
                {
                    break;
                }
                else if (strcmp(message, "Path set") == 0)
                {
                    break;
                }
                else
                {
                    printf("%s\n", message);
                }
                
            }
        }

        if (strcmp(message, "Path set") != 0)
        {
            memset(buffer, '\0', sizeof(buffer));
            memset(message, '\0', sizeof(message));
            printf("Enter options: ");
            fgets(buffer, sizeof(buffer) -1, stdin);
            sscanf(buffer, "%s", message);
            sent_bytes = send(*client_socket, message, sizeof(message) - 1, 0);
            if (sent_bytes == -1)
            {
                fprintf(stderr, "Send() function failed\n");
                close(*client_socket);
                exit(1);
            }
        }
        else
        {
            break;
        }
    }
}

void send_file(int *client_socket, char *file_name)
{
    // Variable declaration section
    FILE *file;
    int size_of_file = 0;
    int success = -1;
    int sent_size = 256;
    int sent_data = -1;
    char buffer[sent_size];
    int total_bytes = 0;
    
    file = fopen(file_name, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "fopen() function failed");
        close(*client_socket);
        exit(1);
    }

    // Determine size of file
    fseek(file, 0, SEEK_END);
    size_of_file = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Sends the name of file to Server
    success = send(*client_socket, file_name, sent_size - 1, 0);
    if (success == -1)
    {
        fprintf(stderr, "Send() function failed");
        close(*client_socket);
        fclose(file);
        exit(1);
    }
    printf("Sent file name: %s to Server\n",file_name);

    set_directory(client_socket);

    // Sends the file size to server
    sprintf(buffer, "%d", size_of_file);
    success = send(*client_socket, buffer, sent_size -1, 0);
    if (success == -1)
    {
        fprintf(stderr, "Send() function failed");
        close(*client_socket);
        fclose(file);
        exit(1);
    }
    printf("Sent file size: %d to Server\n", size_of_file);

    int n = -1;
    while (!feof(file))
    {
        n = fread(buffer, sizeof(char), sent_size - 1, file);
        if (n > 256)
        {
            sent_data = send(*client_socket, buffer, sent_size - 1, 0);
        }
        else
        {
            sent_data = send(*client_socket, buffer, n, 0);
        }
        total_bytes += sent_data;
        bzero(buffer, sent_size);
        if (sent_data == -1)
        {
            fprintf(stderr, "Send() function failed\n");
            close(*client_socket);
            fclose(file);
            exit(1);
        }   
        //printf("Sent %d bytes to Server\n", sent_data);
    }
    printf("Total bytes sent: %d\n", total_bytes);
    printf("File sent successfully! Connection to Server has ended.\n");
    fclose(file);
    close(*client_socket);

}