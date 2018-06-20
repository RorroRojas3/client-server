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
#include "server_setup.h"

#define MAXSIZE 1024

void set_server_info(struct addrinfo *temp_info, struct addrinfo **server_info, int ipv, char *port_number)
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

    // Sets the type of socket to TCP
    temp_info->ai_socktype = SOCK_STREAM;
    
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
        perror("s ocket() function failed ");
        exit(1);
    }

    // Binds Server socket to the defined port
    success = bind(*server_socket, server_info->ai_addr, server_info->ai_addrlen);
    if (success == -1)
    {
        perror("bind() function failed ");
        close(*server_socket);
        exit(1);
    }

    // Frees struct addrinfo "server_info" structure
    freeaddrinfo(server_info);

    // Server set to listen to clients in defined port
    success = listen(*server_socket, max_clients);
    if (success == -1)
    {
        perror("listen() function failed ");
        close(*server_socket);
        exit(1);
    }

    printf("Server waiting for Clients!\n");
}

// Sends the directory to Client
void set_path(char *path, int *client_socket, int client_option, char *file_name)
{
    // Variable Declaration Section
	DIR *directory;
	struct dirent *directory_pointer;
	char client_input[MAXSIZE];
	char buffer[MAXSIZE];
	int sent_bytes = 0;
	int received_bytes = 0;
	int success = -1;
	
	// Clean garbage from string variables
	memset(client_input, '\0', sizeof(client_input));
	memset(buffer, '\0', sizeof(buffer));
	
	// Obtain current working directory
	if (getcwd(path, MAXSIZE) == NULL)
	{
		perror("getcwd() function failed!");
		exit(1);
	}
	
	// Infinite loop which sends Client Sever's files/directories
	while(1)
	{
		// Change the current working directory
		chdir(path);
		
		// Opens current directory based on path given
		directory = opendir(path);
		if (directory ==  NULL)
		{
			perror("opendir() function failed");
			exit(1);
		}
		
		strcpy(buffer, "\nFiles/Directories on current directory:");
		sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
		if (sent_bytes == -1)
		{
			perror("send() function failed");
			exit(1);
		}
		
		// Sends name of files/directories to Client
		while((directory_pointer = readdir(directory)) != NULL)
		{
			if ((directory_pointer->d_type == DT_DIR) || (directory_pointer->d_type == DT_REG))
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, directory_pointer->d_name);
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
			}
		}
		
		// Sends Client different options when receiving file
		if (client_option == 1)
		{
			while ((strcmp(client_input, "y") != 0) && (strcmp(client_input, "n") != 0))
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Save file on this directory? [y/n]");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Server done sending data");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (received_bytes == -1)
				{
					perror("recv() function failed");
					exit(1);
				}
				strcpy(client_input, buffer);
			}
			
			// Client wants to save file on current directory
			if (strcmp(client_input, "y") == 0)
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Path has been set");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				break;
			}
			else
			{
				// Allows Client to create a directory 
				memset(client_input, '\0', sizeof(client_input));
				while((strcmp(client_input, "y") != 0) && (strcmp(client_input,"n") != 0))
				{
					memset(buffer, '\0', sizeof(buffer));
					memset(client_input, '\0', sizeof(client_input));
					strcpy(buffer, "Create directory in current directory? [y/n]");
					sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
					if (sent_bytes == -1)
					{
						perror("send() function failed");
						exit(1);
					}
					
					memset(buffer, '\0', sizeof(buffer));
					strcpy(buffer, "Server done sending data");
					sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
					if (sent_bytes == -1)
					{
						perror("send() function failed");
						exit(1);
					}
					
					memset(buffer, '\0', sizeof(buffer));
					received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
					if (received_bytes == -1)
					{
						perror("recv() function failed");
						exit(1);
					}
					strcpy(client_input, buffer);
				}
				
				if (strcmp(client_input, "y") == 0)
				{
					memset(buffer, '\0', sizeof(buffer));
					strcpy(buffer, "Enter name of directory to be created:");
					sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
					if (sent_bytes == -1)
					{
						perror("send() function failed");
						exit(1);
					}
					
					memset(buffer, '\0', sizeof(buffer));
					strcpy(buffer, "Server done sending data");
					sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
					if (sent_bytes == -1)
					{
						perror("send() function failed");
						exit(1);
					}
					
					memset(buffer, '\0', sizeof(buffer));
					memset(client_input, '\0', sizeof(client_input));
					received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
					if (received_bytes == -1)
					{
						perror("recv() function failed");
						exit(1);
					}
					strcpy(client_input, buffer);
					sprintf(path, "%s/%s", path, client_input);
					
					success = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					if (success == -1)
					{
						perror("mkdir() function failed");
						exit(1);
					}
				}
			}
		}
		
		// Sends Client different options when sending a file
		else if (client_option == 2)
		{
			while ((strcmp(client_input, "y") != 0) && (strcmp(client_input, "n") != 0))
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Send file from this directory? [y/n]");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Server done sending data");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (received_bytes == -1)
				{
					perror("recv() function failed");
					exit(1);
				}
				strcpy(client_input, buffer);
			}
			if (strcmp(client_input, "y") == 0)
			{
				memset(buffer, '\0', sizeof(buffer));
				memset(client_input, '\0', sizeof(client_input));
				strcpy(buffer, "Enter the name of file to be sent:");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Server done sending data");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (received_bytes == -1)
				{
					perror("recv() function failed");
					exit(1);
				}
				strcpy(client_input, buffer);
				strcpy(file_name, client_input);

                memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Path has been set");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
                
				sprintf(path, "%s/%s", path, client_input);
				break;
			}
		}
		
		// Sends Client different options when deleting a file 
		else
		{
			while ((strcmp(client_input, "y") != 0) && (strcmp(client_input, "n") != 0))
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Delete file from this directory? [y/n]");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Server done sending data");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (received_bytes == -1)
				{
					perror("recv() function failed");
					exit(1);
				}
				strcpy(client_input, buffer);
			}
			
			if (strcmp(client_input, "y") == 0)
			{
				memset(buffer, '\0', sizeof(buffer));
				memset(client_input, '\0', sizeof(client_input));
				strcpy(buffer, "Enter the name of file to be deleted:");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Server done sending data");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				memset(buffer, '\0', sizeof(buffer));
				received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
				if (received_bytes == -1)
				{
					perror("recv() function failed");
					exit(1);
				}
				strcpy(client_input, buffer);
                strcpy(file_name, client_input);

                memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "Path has been set");
				sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
				if (sent_bytes == -1)
				{
					perror("send() function failed");
					exit(1);
				}
				
				sprintf(path, "%s/%s", path, client_input);
				break;
			}
		}
		
		// Goes to another directory
		memset(buffer, '\0', sizeof(buffer));
		memset(client_input, '\0', sizeof(client_input));
		strcpy(buffer, "Enter name of directory you want to go next:");
		sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
        if (sent_bytes == -1)
        {
            perror("send() function failed");
            exit(1);
        } 
        
        memset(buffer, '\0', sizeof(buffer));
		strcpy(buffer, "Server done sending data");
		sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1,  0);
		if (sent_bytes == -1)
		{
			perror("send() function failed");
			exit(1);
		}
		
		memset(buffer, '\0', sizeof(buffer));
		received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
		if (received_bytes == -1)
		{
			perror("recv() function failed");
			exit(1);
		}
		strcpy(client_input, buffer);
		sprintf(path, "%s/%s", path, client_input);	
	}
}

// Receives a file from the Client
void receive_file_from_client(int *client_socket, int client_option)
{
    // Variable Declaration Section
    char file_name[MAXSIZE];
    char path[MAXSIZE];
    char buffer[MAXSIZE];
    int received_bytes = -1;
    int file_size;
    int total_bytes = 0;

    // Clear string variables
    memset(file_name, '\0', sizeof(file_name));
    memset(path, '\0', sizeof(path));
    memset(buffer, '\0', sizeof(buffer));

    // Receives name of file
    received_bytes = recv(*client_socket, buffer, MAXSIZE - 1, 0);
    if (received_bytes == -1)
    {
        perror("recv() function failed");
        exit(1);
    }
    sprintf(file_name, "%s", buffer);
    printf("Name of file to be received: %s\n", file_name);

    // Lets client decide path for file to stored at
    set_path(path, client_socket, client_option, file_name);
    sprintf(path, "%s/%s", path, file_name);

    // Receives the file size 
    memset(buffer, '\0', sizeof(buffer));
    received_bytes = recv(*client_socket, buffer, MAXSIZE - 1, 0);
    if (received_bytes == -1)
    {
        perror("recv() function failed");
        exit(1);
    }
    file_size = atoi(buffer);
    printf("Size of file to be received: %d\n", file_size);

    // Receives file from Client
    FILE *file;
    file = fopen(path, "wb");
    while (received_bytes != 0)
    {
        memset(buffer, '\0', sizeof(buffer));
        received_bytes = recv(*client_socket, buffer, MAXSIZE - 1, 0);
        if (received_bytes == -1)
        {
            perror("recv() function failed");
            fclose(file);
            exit(1);
        }
        
        if (received_bytes != 0)
        {
            total_bytes += received_bytes;
            fwrite(buffer, sizeof(char), received_bytes, file);
        }
    }

    // Closes accepted client socket
    //printf("Bytes received: %d\n", total_bytes);
    //printf("Successfull file transmission! Client left!\n");
    fclose(file);
}

void send_file_to_client(int *client_socket, int client_option)
{
    // Variable Declaration Section
    FILE *file;
    char buffer[MAXSIZE];
    char path[MAXSIZE];
    char file_name[MAXSIZE];
    int size_of_file = 0;
    int sent_bytes = 0;
    int bytes = 0;
    int total_bytes = 0;
    
    memset(path, '\0', sizeof(path));
    memset(file_name, '\0', sizeof(path));

    set_path(path, client_socket, client_option, file_name);

    // Open file chosen by the Client
    file = fopen(path, "rb");
    if (file == NULL)
    {
        perror("fopen() function failed");
        exit(1);
    }

    // Determine size of file
    fseek(file, 0, SEEK_END);
    size_of_file = ftell(file);
    fseek(file, 0 , SEEK_SET);

    // Send the name of file to Client
    sent_bytes = send(*client_socket, file_name, sizeof(file_name) - 1, 0);
    if (sent_bytes == -1)
    {
        perror("send() function failed");
        fclose(file);
        exit(1);
    }
    //printf("Sent file name: %s to Client\n", file_name);

    // Send file size to Client
    sprintf(buffer, "%d", size_of_file);
    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
    if (sent_bytes == -1)
    {
        perror("send() function failed");
        fclose(file);
        exit(1);
    }
    //printf("Sent file size: %d to Client\n", size_of_file);

    while(!feof(file))
    {
        memset(buffer, '\0', sizeof(buffer));
        bytes = fread(buffer, sizeof(char), sizeof(buffer) - 1, file);
        
        sent_bytes = send(*client_socket, buffer, bytes, 0);
        if (sent_bytes == -1)
        {
        	perror("send() function failed");
        	fclose(file);
        	exit(1);
        }
        total_bytes += sent_bytes;
    }

    //printf("Total bytes sent to Client: %d\n", total_bytes);
    //printf("File sent successfully! Connection to Client has ended!\n");
    fclose(file);
}

// Deletes a file/directory 
void delete_file(int *client_socket, int client_option)
{
    // Variable declaration section
    char path[MAXSIZE];
    char file_name[MAXSIZE];
    char buffer[MAXSIZE];
    int sent_bytes = -1;

    memset(path, '\0', sizeof(path));
    memset(file_name, '\0', sizeof(file_name));
	memset(buffer, '\0', sizeof(buffer));

    set_path(path, client_socket, client_option, file_name);
    remove(path);
    
    sprintf(buffer, "File has been deleted: %s", file_name);
    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
    if (sent_bytes == -1)
    {
    	perror("send() function failed");
    	exit(1);
    }
    
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

// Sever starts and accepts up to 10 clients at once
void accept_clients(int *server_socket, int *client_socket)
{
    // Variable declaration section
    struct sockaddr_storage client_address;
    socklen_t ip_length;
    char ip[INET6_ADDRSTRLEN];
    char buffer[MAXSIZE];
    int child = -1;
    int received_bytes = -1; 
    int sent_bytes = -1;
    int received_input = 0;

    while(1)
    {
        // Server set to accept up to 10 different clients
        ip_length = sizeof(client_address);
        *client_socket = accept(*server_socket, (struct sockaddr *)&client_address, &ip_length);
        if (*client_socket == -1)
        {
            perror("accept() function failed");
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
            
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, "\nSelect an option:\n1)Send a file\n2)Receive a file\n3)Delete a file\n4)Exit program");
            sent_bytes = send(*client_socket, buffer, MAXSIZE - 1, 0);
            if (sent_bytes == -1)
            {
                perror("send() function failed");
                close(*client_socket);
                exit(1);
            } 

            // Server receives what Client desided to do
            received_bytes = recv(*client_socket, buffer, MAXSIZE - 1,  0);
            if (received_bytes == -1)
            {
                perror("recv() function failed");
                close(*client_socket);
                exit(1);
            }
            received_input = atoi(buffer);


            // Receive a file
            if (received_input == 1)
            {
                receive_file_from_client(client_socket, received_input);
                close(*client_socket);
                exit(1);
            }
            // Send a file
            else if (received_input == 2)
            {
                send_file_to_client(client_socket, received_input);
                close(*client_socket);
                exit(1);

            }
            // Delete a file
            else if (received_input == 3)
            {
                delete_file(client_socket, received_input);
                close(*client_socket);
                exit(1);
            }
            // Exit
            else 
            {
                close(*client_socket);
                printf("Successfull exit! Client left\n");
                exit(1);
            }
            
        }
        // Closes accepted client socket
        close(*client_socket);
    }
}



