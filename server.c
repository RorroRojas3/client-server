#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "4380" //  Internet port that users will connect to
#define BACKLOG 10 // Number of clients Server can hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so it must be restored:
    int savedError = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = savedError;
}

// Get socket address, IPV4 or IPV6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, new_fd; // Listen to sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Client's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

	// Clears all data for structure hints
    memset(&hints, 0, sizeof(hints));
    // Sets IPv family either to IPv4 or IPv6
    hints.ai_family = AF_UNSPEC;
    // Sock type sets as TCP
    hints.ai_socktype = SOCK_STREAM;
    // Use my IP address
    hints.ai_flags = AI_PASSIVE;

	// All server information will be saved to pointer "servinfo"
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop thorugh all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
    	// Returns socket descriptor of Server
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("Server: socket");
            continue;
        }

		// Reuse desired port
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }
		
		// Bind to desired port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("Server: bind");
            continue;
        }

        break;
    }

	// Free server structure
    freeaddrinfo(servinfo); 

	// If P reaches end of Linked List, it will be equal to NULL, therefore
	// it means that the "bind()" was not successfull
    if (p == NULL)
    {
        fprintf(stderr, "Server: failed to bind\n");
        exit(1);
    }

	// Server sets to "listen" to clients
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("Listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // Kill all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("Sigaction");
        exit(1);
    }

    printf("Server: Waiting for connections ... \n");

    // Main accept loop
    while(1)
    {
    	// Sets size to size of Client's address
        sin_size = sizeof(their_addr);
        // Client
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        // Checks if "accept()" function resolved succesfully
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

		// Calls function inet_ntop() to acquire Client's IP information
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("Server: Got connection from %s\n", s);

        // Creates a child process
        if (!fork())
        {
        	// Child does not need Server socket
            close(sockfd); 
            // Sends string to client and checks if sent successfully
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
            {
                perror("Send");
            } 
            // Closes child socket
            close(new_fd);
            // Child exits
            exit(0);
        }
        // Parent closes child process 
        close(new_fd); 
    }

	// Program terminates
    return 0;
}
