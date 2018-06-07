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

// Sends the directory to Client
void display_directories(char *path, int *client_socket)
{
    // Variable Declaration Section
	DIR *directory;
	struct dirent *directory_pointer;
    char input[1024];
    char original_directory[1024];
    char buffer[1024];
    char input_command[2];
    int original_path_length = 0;
    int c1 = 0;
    int verify = 0;
    int sent_bytes = -1;
    int received_bytes = -1;
    int success = -1;

    // Clear garbage from character strings
    memset(input, '\0', sizeof(input));
    memset(input_command, '\0', sizeof(input_command));
    memset(original_directory, '\0', sizeof(original_directory));
    memset(buffer, '\0', sizeof(buffer));

    // Obtain current working directory which will be the restricted
    // directory in which files can be stored at
    getcwd(original_directory, sizeof(original_directory));
    original_path_length = strlen(original_directory);
    strcpy(path, original_directory);
    
    // Infinite loop that will be broken of when the user chooses
    // a directory
    while(1)
    {
        // Changes the current working directory
        chdir(path);

        // Obtains the new current directory and stores it on
        // variable "path"
        getcwd(buffer, sizeof(buffer));
        printf("Original: %s\n", original_directory);
        printf("Current directory: %s\n", buffer);
        
        // Checks if the new path can be acccessed for security reasons
        for (c1 = 0; c1 < original_path_length; c1++)
        {
            if (buffer[c1] == original_directory[c1])
            {
                verify++;
            }
        }

        // If path can be accessed
        if (verify == original_path_length)
        {
            // Resets value of verification variable
            verify = 0;

            directory = opendir(path);
            if (directory ==  NULL)
            {
                perror("Cannot open directory");
                exit(1);
            }

            strcpy(buffer, "\nFiles/Directories of current location:");
            sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
            if (sent_bytes == -1)
            {
                fprintf(stderr, "Send() function failed");
                close(*client_socket);
                exit(1);
            }
            // Prints the directories inside the default directory
            while((directory_pointer = readdir(directory)) != NULL)
            {
                if ((directory_pointer->d_type == DT_DIR) || directory_pointer->d_type == DT_REG)
                {
                    sent_bytes = send(*client_socket, directory_pointer->d_name, 1023, 0);
                    if (sent_bytes == -1)
                    {
                        fprintf(stderr, "Send() function failed");
                        close(*client_socket);
                        exit(1);
                    }
               }
            }

            // Asks the user if to chose current directory to save file
            while((strcmp(input_command, "Y") != 0) && (strcmp(input_command, "N") != 0))
            {
                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, "Save file on this directory? [Y/N]");
                sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                if (sent_bytes == -1)
                {
                    fprintf(stderr, "Send() function failed");
                    close(*client_socket);
                    exit(1);
                }

                // Server stops sending data
                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, "Done");
                sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                if (sent_bytes == 1)
                {
                    fprintf(stderr, "Send() function failed\n");
                    close(*client_socket);
                    exit(1);
                }

                memset(buffer, '\0', sizeof(buffer));
                received_bytes = recv(*client_socket, buffer, 1024 - 1, 0);
                if (received_bytes == -1)
                {
                    fprintf(stderr, "Recv() function failed");
                    close(*client_socket);
                    exit(1);
                }
                strcpy(input_command, buffer);
            }

            // Program will proceed and return the current path
            if (strcmp(input_command, "Y") == 0)
            {
                // Server stops sending data
                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, "Path set");
                sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                if (sent_bytes == 1)
                {
                    fprintf(stderr, "Send() function failed\n");
                    close(*client_socket);
                    exit(1);
                }
                break;
            }

            // Asks the user to create directory in currrent folder or 
            // go to next directory available
            else
            {
                memset(input_command, '\0', sizeof(input_command));
                while((strcmp(input_command, "Y") != 0) && (strcmp(input_command, "N") != 0))
                {
                    memset(buffer, '\0', sizeof(buffer));
                    memset(input_command, '\0', sizeof(input_command));
                    strcpy(buffer, "Create directory in current directory? [Y/N]: ");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == -1)
                    {
                        fprintf(stderr, "Send() function failed");
                        close(*client_socket);
                        exit(1);
                    }

                    // Server stops sending data
                    memset(buffer, '\0', sizeof(buffer));
                    strcpy(buffer, "Done");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == 1)
                    {
                        fprintf(stderr, "Send() function failed\n");
                        close(*client_socket);
                        exit(1);
                    }

                    memset(buffer, '\0', sizeof(buffer));
                    received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (received_bytes == -1)
                    {
                        fprintf(stderr, "Recv() function failed");
                        close(*client_socket);
                        exit(1);
                    }
                    strcpy(input_command, buffer);
                }

                // Creates directory if user chose to
                if (strcmp(input_command, "Y") == 0)
                {
                    char directory_name[1024];
                    memset(buffer, '\0', sizeof(buffer));
                    strcpy(buffer, "Enter name of directory: ");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == -1)
                    {
                        fprintf(stderr, "Send() function failed");
                        close(*client_socket);
                        exit(1);
                    }

                    // Server stops sending data
                    memset(buffer, '\0', sizeof(buffer));
                    strcpy(buffer, "Done");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == 1)
                    {
                        fprintf(stderr, "Send() function failed\n");
                        close(*client_socket);
                        exit(1);
                    }

                    memset(buffer, '\0', sizeof(buffer));
                    memset(input, '\0', sizeof(directory_name));
                    received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (received_bytes == -1)
                    {
                        fprintf(stderr, "Recv() function failed");
                        close(*client_socket);
                        exit(1);
                    }
                    strcpy(directory_name, buffer);
                    printf("%s\n", path);
                    printf("%s\n", directory_name);
                    sprintf(path, "%s/%s", path, directory_name);
                    printf("%s\n", path);

                    success = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    if (success == -1)
                    {
                        perror("Mkdir() function failed");
                        exit(1);
                    }
                    memset(input_command, '\0', sizeof(input_command));
                }
                // Asks user to go to another directory
                else
                {
                    memset(buffer, '\0', sizeof(buffer));
                    memset(input, '\0', sizeof(input));
                    strcpy(buffer, "Enter the name of directory you want to go: ");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == -1)
                    {
                        fprintf(stderr, "Send() function failed");
                        close(*client_socket);
                        exit(1);
                    }

                    // Server stops sending data
                    memset(buffer, '\0', sizeof(buffer));
                    strcpy(buffer, "Done");
                    sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (sent_bytes == 1)
                    {
                        fprintf(stderr, "Send() function failed\n");
                        close(*client_socket);
                        exit(1);
                    }

                    memset(buffer, '\0', sizeof(buffer));
                    memset(input, '\0', sizeof(input));
                    received_bytes = recv(*client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (received_bytes == -1)
                    {
                        fprintf(stderr, "Recv() function failed");
                        close(*client_socket);
                        exit(1);
                    }
                    strcpy(input, buffer);
                    sprintf(path, "%s/%s", path, input);
                }
                memset(input_command, '\0', sizeof(input_command));
            }
        }
        else
        {
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, "\nDirectory cannot be accessed!");
            sent_bytes = send(*client_socket, buffer, sizeof(buffer) -1, 0);
            if (sent_bytes == -1)
            {
                fprintf(stderr, "Send() function failed!");
                close(*client_socket);
                exit(1);
            }
            strcpy(path, original_directory);
            verify = 0;
        }
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
    char path[receive_size];
    int file_size = 0;
    int bytes_received = -1; 
    int total_bytes = 0;

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
            sprintf(file_name, "%s", buffer);
            printf("Name of file to be received: %s\n", file_name);

            display_directories(path, client_socket);

            sprintf(path, "%s/%s", path, file_name);
            //printf("Path to Received file: %s\n", path);

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
            file = fopen(path, "wb");
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
                
                if (bytes_received != 0)
                {
                    total_bytes += bytes_received;
                    fwrite(buffer, sizeof(char), bytes_received, file);
                }
                
                bzero(buffer, receive_size);
                //printf("Bytes received: %d\n", bytes_received);
            }
            // Closes accepted client socket
            printf("Bytes received: %d\n", total_bytes);
            printf("Successfull file transmission! Client left!\n");
            close(*client_socket);
            fclose(file);
            exit(1);
        }
        // Closes accepted client socket
        close(*client_socket);
    }
}

// Deletes a file/directory 
void delete_file(int *client_socket, char *file_name)
{
    // Variable Declaration Section
	DIR *directory;
	struct dirent *directory_pointer;
    char input[1024];
    char original_directory[1024];
    char buffer[1024];
    char input_command[2];
    int original_path_length = 0;
    int c1 = 0;
    int verify = 0;
    int sent_bytes = -1;
    int received_bytes = -1;
    int success = -1;

    // Clear garbage from character strings
    memset(input, '\0', sizeof(input));
    memset(input_command, '\0', sizeof(input_command));
    memset(original_directory, '\0', sizeof(original_directory));
    memset(buffer, '\0', sizeof(buffer));

    // Obtain current working directory which will be the restricted
    // directory in which files can be stored at
    getcwd(original_directory, sizeof(original_directory));
    original_path_length = strlen(original_directory);
    strcpy(path, original_directory);
    
    // Infinite loop that will be broken of when the user chooses
    // a directory
    while(1)
    {
        // Changes the current working directory
        chdir(path);

        // Obtains the new current directory and stores it on
        // variable "path"
        getcwd(buffer, sizeof(buffer));
        printf("Original: %s\n", original_directory);
        printf("Current directory: %s\n", buffer);
        
        // Checks if the new path can be acccessed for security reasons
        for (c1 = 0; c1 < original_path_length; c1++)
        {
            if (buffer[c1] == original_directory[c1])
            {
                verify++;
            }
        }

        // If path can be accessed
        if (verify == original_path_length)
        {
            // Resets value of verification variable
            verify = 0;

            directory = opendir(path);
            if (directory ==  NULL)
            {
                perror("Cannot open directory");
                exit(1);
            }

            strcpy(buffer, "\nFiles/Directories of current location:");
            sent_bytes = send(*client_socket, buffer, sizeof(buffer) - 1, 0);
            if (sent_bytes == -1)
            {
                fprintf(stderr, "Send() function failed");
                close(*client_socket);
                exit(1);
            }
            // Prints the directories inside the default directory
            while((directory_pointer = readdir(directory)) != NULL)
            {
                if ((directory_pointer->d_type == DT_DIR) || (directory_pointer->d_type == DT_REG))
                {
                    sent_bytes = send(*client_socket, directory_pointer->d_name, 1023, 0);
                    if (sent_bytes == -1)
                    {
                        fprintf(stderr, "Send() function failed");
                        close(*client_socket);
                        exit(1);
                    }
               }
            }
       }
       else
       {
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, "\nDirectory cannot be accessed!");
            sent_bytes = send(*client_socket, buffer, sizeof(buffer) -1, 0);
            if (sent_bytes == -1)
            {
                fprintf(stderr, "Send() function failed!");
                close(*client_socket);
                exit(1);
            }
            strcpy(path, original_directory);
            verify = 0;
       }
         
}

