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
#include <sys/stat.h>
#include <dirent.h>
#include "client_setup.h"

#define MAXSIZE 1024

void set_client_info(struct addrinfo *temp_info, struct addrinfo **server_info, int ipv, char *server_address, char *port_number)
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

    // Sets TCP socket
    temp_info->ai_socktype = SOCK_STREAM;	

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
    char buffer[MAXSIZE];
    char client_input[MAXSIZE];
    int received_bytes = 0;
    int sent_bytes = 0;
    int client_option = 0;
    
    // Clears garbage from String variables
    memset(ip, '\0', sizeof(ip));
    memset(buffer, '\0', sizeof(buffer));
    

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
    
    // Receives and sends option of Sending/Receiving/Deleting/Exiting program from Server
    received_bytes = recv(*client_socket, buffer, sizeof(buffer) -1, 0);
    if (received_bytes == -1)
    {
    	fprintf(stderr, "recv() function failed");
    	close(*client_socket);
    	exit(1);
    }
    printf("Server sent: %s", buffer);
    while((strcmp(client_input, "1") != 0) && (strcmp(client_input, "2") != 0) && (strcmp(client_input, "3") != 0) && (strcmp(client_input, "4") != 0))
    {
    	memset(buffer, '\0', sizeof(buffer));
    	printf("Enter command: ");
    	fgets(buffer, sizeof(buffer), stdin);
    	sscanf(client_input, "%s", buffer);
    }
    sent_bytes = send(*client_socket, client_input, sizeof(client_input) - 1, 0);
    if (sent_bytes == -1)
    {
    	fprintf(stderr, "send() function failed");
    	close(*client_socket);
    	exit(1);
    }
    
   	// Based on Client input, Client will either Send/Receive/Delete or Exit program 
    client_option = atoi(client_input);
    if (client_option == 1)
    {
    	send_file_to_server(client_socket);
    	close(*client_socket);
    	exit(1);
    }
    else if (client_option == 2)
    {
    	receive_file_from_server(client_socket);
    	close(*client_socket);
    	exit(1);
    }
    else if (client_option == 3)
    {
    	set_path(client_socket);
    	close(*client_socket);
    	exit(1);
    }
    else
    {
    	close(*client_socket);
    	printf("Program will be terminated!");
    	exit(1);
    }
}

// Function lets the user set the directory in which file will be saved
// User will be allowed to create directories and save the files in directory preferred
void set_path(int *client_socket)
{
    int received_bytes = 1;
    int sent_bytes = -1;
    char buffer[MAXSIZE];
    char message[MAXSIZE];

    while(1)
    {
        // Infinite loop which will receive messages from the Server and
        // display it to the user and user allowed to sent a reponse
        // to server based on what Server asks
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
                if(strcmp(message, "Sever done sending data") == 0)
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

        // If the directory has not been set, the user will be allowed to send 
        // command to server regarding option on creation/selection of directory
        if (strcmp(message, "Path set") != 0)
        {
            memset(buffer, '\0', sizeof(buffer));
            memset(message, '\0', sizeof(message));
            printf("Enter option: ");
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
        // If path has been set for file to be transferred, breaks from infinite while loop
        else
        {
            break;
        }
    }
}

void choose_path(char *path, char *file_name, int client_option)
{
	// Variable Declaration Section
	DIR *directory;
	struct dirent *directory_pointer;
	char buffer[MAXSIZE];
	char client_input[MAXSIZE];
	
	// Clean garbage from string variables
	memset(buffer, '\0', sizeof(buffer));
	memset(client_input, '\0', sizeof(client_input));
	
	if (getcwd(path, sizeof(path)) == NULL)
	{
		perror("getcwd() function failed!");
		exit(1);
	}
	
	while(1)
	{
		// Change current directory
		chdir(path);
		
		// Opens current directory based on the path given
		directory = opendir(path);
		if (directory == NULL)
		{
			perror("opendir() function failed!");
			exit(1);
		}
		
		printf("\nFiles/Directories on current directory:\n\n");
		
		while((directory_pointer = readdir(directory)) != NULL)
		{
			if ((directory_pointer->d_type == DT_DIR) || (directory_pointer->d_type == DT_REG))
			{
				printf("%s\n", directory_pointer->d_name);
			}
		}
		printf("\n");
	
	
		// Lets Client choose file to be sent to Server
		if (client_option == 1)
		{
			while((strcmp(client_input, "y") != 0) && (strcmp(client_input, "n") != 0))
			{
				memset(buffer, '\0', sizeof(buffer));
				memset(client_input, '\0', sizeof(client_input));
				printf("Choose file from current directory? [y/n]: ");
				fgets(buffer, sizeof(buffer), stdin);
				sscanf(client_input, "%s", buffer);
			}
			if (strcmp(client_input, "y") == 0)
			{
				memset(buffer, '\0', sizeof(buffer));
				memset(client_input, '\0', sizeof(client_input));
				printf("Enter name of file: ");
				fgets(buffer, sizeof(buffer), stdin);
				sscanf(client_input, "%s", buffer);
				strcpy(file_name, client_input);
				break;
			}
		}
		// Lets Client choose directory to save file from Server
		else
		{
			while((strcmp(client_input, "y") != 0) && (strcmp(client_input, "n") != 0))
			{
				memset(buffer, '\0', sizeof(buffer));
				memset(client_input, '\0', sizeof(client_input));
				printf("Save file on current directory? [y/n]: ");
				fgets(buffer, sizeof(buffer), stdin);
				sscanf(client_input, "%s", buffer);
			}
			if (strcmp(client_input, "y") == 0)
			{
				break;
			}
		}
		// Goes to another directory
		memset(buffer, '\0', sizeof(buffer));
		memset(client_input, '\0', sizeof(client_input));
		printf("\nEnter name of directory to go next: ");
		fgets(buffer, sizeof(buffer), stdin);
		sscanf(client_input, "%s", buffer);
		sprintf(path, "%s/%s", path, client_input);
	}
}

void send_file_to_server(int *client_socket)
{
    // Variable declaration section
    FILE *file;
    int size_of_file = 0;
    int success = -1;
    int sent_size = 256;
    int sent_data = -1;
    char buffer[sent_size];
    int total_bytes = 0;
    char file_name[MAXSIZE];
    
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

    set_path(client_socket);

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
        if (n > MAXSIZE)
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

void receive_file_from_server(int *client_socket)
{

}
