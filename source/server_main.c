/*
Sets up a listener socket on port 47321 and runs a server hosting this online game
If you want people to be able to join over internet, you probably have to like,
open that port or something, idk.

Players should join using the client program (compiled from main.c in this folder)
To join, for example, if you're on the same computer, put localhost for the server name.
Or put the IP of your friend's VPS where he's running this program or whatever.
If he has a DNS name for his server, you should be able to put that too.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Unix standard library. The server code is not portable and must run on a Unix-like system.
// (Although the client can still be compiled for wincux and wincux can join our server too)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <time.h>
#include <stdbool.h>

// Port we're listening on. Because 47321 -> 4V3R1 -> AVERI.
#define PORT "47321"

#define FLAG_GEO 1

// Convert socket to IP address string.
// addr: struct sockaddr_in or struct sockaddr_in6
const char *inet_ntop2(void *addr, char *buf, size_t size)
{
	struct sockaddr_storage *sas = addr;
	struct sockaddr_in *sa4;
	struct sockaddr_in6 *sa6;
	void *src;

	switch (sas->ss_family) {
		case AF_INET:
			sa4 = addr;
			src = &(sa4->sin_addr);
			break;
		case AF_INET6:
			sa6 = addr;
			src = &(sa6->sin6_addr);
			break;
		default:
			return NULL;
	}

	return inet_ntop(sas->ss_family, src, buf, size);
}

// Return a listening socket.
int get_listener_socket(void)
{
	int listener;	 // Listening socket descriptor
	int yes=1;		// For setsockopt() SO_REUSEADDR, below
	int rv;

	struct addrinfo hints, *ai, *p;

	// Get us a socket and bind it
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // IPv4, which, for a server, is to be preffered over IPv6
	hints.ai_socktype = SOCK_STREAM; // Stream-type socket
	hints.ai_flags = AI_PASSIVE; // Local host's (The machine this program is running on) IP.
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next)
	{
		// socket(): get the file descriptor
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// Lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		// bind(): take ownership of the port
		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// If p is still NULL here, it means we didn't get bound
	if (p == NULL) {
		return -1;
	}

	freeaddrinfo(ai); // All done with this

	// listen(): set up listener which will queue up incoming connections to be accept()ed
	if (listen(listener, 10) == -1) {
		return -1;
	}

	return listener;
}

#include "server_game.c"

// Process all existing connections.
void process_connections(int listener, int *fd_count, int *fd_size, struct pollfd *pfds)
{
	for(int i = 0; i < *fd_count; i++)
	{
		// Check if someone's ready to read (or closed their connection)
		if ((pfds)[i].revents & (POLLIN | POLLHUP))
		{
			if (pfds[i].fd == listener) // If the fd is the listener, it's a new connection
			{
				struct sockaddr_storage remoteaddr; // Client address
				socklen_t addrlen;
				int newfd;  // Newly accept()ed socket descriptor
				char remoteIP[INET6_ADDRSTRLEN];

				addrlen = sizeof remoteaddr;
				newfd = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);

				if (newfd == -1) {
					perror("accept");
				} else { // Add newfd to pfds
					// If we don't have room, add more space in the pfds array
					if (*fd_count == *fd_size) {
						*fd_size *= 2; // Double it
						pfds = realloc(pfds, sizeof(struct pollfd) * (*fd_size));
						if (pfds < 0)
							perror("realloc");
						printf("Doubled *fd_size, its now %i\n", *fd_size);
					}
					pfds[*fd_count].fd = newfd;
					pfds[*fd_count].events = POLLIN; // Check ready-to-read
					pfds[*fd_count].revents = 0;
					
					(*fd_count)++;
					
					printf("pollserver: new connection from %s on socket %d\n",
							inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
							newfd);
					
					server_user_connected(newfd);
				}
				
				
			}
			else // If the fd isn't the listener, this is data from one of the connected players
			{
				char buf[256]; // Buffer for client data

				int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);

				int sender_fd = pfds[i].fd;

				if (nbytes <= 0) // Got error or connection closed by client
				{
					if (nbytes == 0) {
						// Connection closed
						printf("pollserver: socket %d hung up\n", sender_fd);
					} else {
						perror("recv");
					}
					
					server_user_disconnected(sender_fd);
					
					close(pfds[i].fd); // Bye!
					
					// Copy the fd from the end of pfds into this place
					pfds[i] = pfds[*fd_count-1];
					(*fd_count)--;
					
					// Have the for-loop check this pfds-index again
					i--; // (Since it's now occupied by a different connection)

				}
				else // We got some good data from a client
				{
					server_got_data(buf, nbytes, sender_fd);
				}
			}
		}
	}
}

// Main: create a listener and connection set, loop forever processing connections.
int main(void)
{
	int listener; // Listening socket descriptor
	
	// Start off with room for 5 connections
	// (We'll realloc as necessary) EMPIRICALLY THAT DOESN'T WORK!!! It gets stuck for some reason???
	//int fd_size = 5;
	int fd_size = 25;
	int fd_count = 0;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
	
	// Set up and get a listening socket
	listener = get_listener_socket();
	
	if (listener == -1) {
		fprintf(stderr, "error getting listening socket\n");
		exit(1);
	}
	
	// Add the listener to set;
	pfds[0].fd = listener;
	// Only report POLLIN ("ready to read on incoming connection")
	pfds[0].events = POLLIN;
	
	fd_count = 1; // (Since we just added the listener)
	
	puts("pollserver: waiting for connections...");
	
	server_init();
	
	// Main loop
	while(1)
	{
		// poll(): the OS will block here until any of the (struct pollfd)s passed to it reports an event
		int poll_count = poll(pfds, fd_count, 2500); // Timeout after 2.500 seconds though
		
		if (poll_count == -1) {
			perror("poll");
			exit(1);
		}
		
		if (poll_count == 0)
		{
			server_tick();
		}
		else
		{
			// Run through connections looking for data to read
			process_connections(listener, &fd_count, &fd_size, pfds);
			
			server_tick();
		}
	}
	
	free(pfds);
}
