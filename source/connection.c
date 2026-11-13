
#ifdef _WIN32
    // Because windows uses these 3 things named the same as raylib stuff,
    // we have to do this to avoid thing-already-declared errors
	#define CloseWindow CloseWindowWin32
	#define Rectangle RectangleWin32
	#define ShowCursor ShowCursorWin32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #undef CloseWindow
    #undef Rectangle
    #undef ShowCursor
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#include <string.h>
#include <fcntl.h>

/* Try to connect to server using data entered in connection menu:

// SERVER ADDRESS STRING
const int conn_sstr_max_len = 40; // (Big enough to accomodate a full IPv6 address)
char conn_sstr[41]; // (That, plus one for \0)
int conn_sstr_len = 0;

// SERVER PORT # STRING
const int conn_pstr_max_len = 5; // Big enough to accomodate the highest valid port number (65535)
char conn_pstr[6]; // Big enough for the longest possible nul-terminated string "65535\0"
int conn_pstr_len = 0;
*/

#define MAXDATASIZE 100 // Maximum number of bytes we can get at once

bool connection_splashed = false; // Has the connection_attempt frame changed the screen yet?
int sockfd; // Socket file descriptor. close() it when you leave the server.

//struct pollfd server_pfd; // A struct used to poll(), which will be set right after sockfd
// Data used to select(), I'm switching over from poll() to select():
fd_set readfds;
struct timeval tv;

char conn_menu_err_desc[100];
int conn_menu_err_strlen = 0;


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void connection_attempt_frame(void)
{
	if (!connection_splashed) {
		add_text_renderends( "Attempting to connect...", 24, 150, 120, 6.0f );
		connection_splashed = true;
		return;
	}
	
	#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
		snprintf(conn_menu_err_desc, conn_menu_err_strlen, "%d", result);
        state = CONNECTION_FAILED_STATE;
        return;
    }
	#endif
	
	int numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	
	// Null-terminate and pad all the strings we're using
	for(int i = conn_sstr_len; i < conn_sstr_max_len+1; i++) {
		conn_sstr[i] = '\0';
	}
	for(int i = conn_pstr_len; i < conn_pstr_max_len+1; i++) {
		conn_pstr[i] = '\0';
	}
	if (player_name_len == 0) {
		player_name[0] = 'F';
		player_name[1] = 'u';
		player_name[2] = 'r';
		player_name[3] = 'c';
		player_name[4] = 'h';
		player_name[5] = 'u';
		player_name[6] = 'd';
		player_name_len = 7;
	}
	for(int i = player_name_len; i < player_name_max_len+1; i++) {
		player_name[i] = '\0';
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((rv = getaddrinfo(conn_sstr, conn_pstr, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		for(int i = 0; i < 40;  i++) {
			if ((conn_menu_err_desc[i] = gai_strerror(rv)[i]) == '\0') {
				conn_menu_err_strlen = i;
				break;
			}
		}
		state = CONNECTION_FAILED_STATE;
		connection_splashed = false;
		return;
	}
	
	// loop through all the results and connect to the first we can, setting sockfd
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
		{
			perror("client: socket");
			#ifdef _WIN32
			snprintf(conn_menu_err_desc, conn_menu_err_strlen, "%d", WSAGetLastError());
			#else
			for(int i = 0; i < 40;  i++) {
				if ((conn_menu_err_desc[i] = strerror(errno)[i]) == '\0') {
					conn_menu_err_strlen = i;
					break;
				}
			}
			#endif
			continue;
		}
		
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
		printf("client: attempting connection to %s\n", s);
		
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR)
		{
			perror("client: connect");
			#ifdef _WIN32
			snprintf(conn_menu_err_desc, conn_menu_err_strlen, "%d", WSAGetLastError());
			WSACleanup();
			#else
			for(int i = 0; i < 40;  i++) {
				if ((conn_menu_err_desc[i] = strerror(errno)[i]) == '\0') {
					conn_menu_err_strlen = i;
					break;
				}
			}
			close(sockfd);
			#endif
			continue;
		}
		
		// Now we make it a non-blocking socket.
		#ifdef _WIN32
		u_long mode = 1;  // 1 = non-blocking, 0 = blocking
		if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
			printf("ioctlsocket failed: %d\n", WSAGetLastError());
		}
		#else
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
		#endif
		/* (From Beej's Guide to Network Programming)
		By setting a socket to non-blocking, you can effectively “poll” the socket for information.
		If you try to read from a non-blocking socket and there’s no data there, it’s not allowed to block,
		so it will return -1 and errno will be set to EAGAIN or EWOULDBLOCK.
		(The specification doesn’t actually specify which your system will return, so check them both.)
		
		This is normally "bad" for a program looping and checking, as it would hog CPU looping as fast as it can,
		but in my case I think it'll be OK since my program already limits itself to ticking at 30FPS
		*/
		
		break;
	}
	
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		state = CONNECTION_FAILED_STATE;
		connection_splashed = false;
		return;
	}
	
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connected to %s\n", s);
	
	freeaddrinfo(servinfo); // all done with this structure
	
	// Now set up the polling struct
	//server_pfd.fd = sockfd;
	//server_pfd.events = POLLIN | POLLHUP;
	//listen(sockfd, 100); // Use the socket, keep backlog of 100
	// Now setup the select data (I'm switching from poll() to select())
	tv.tv_sec = 0;
	tv.tv_usec = 3300000; // (330k microseconds = 0.33 seconds)
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds); // Add sockfd to readfds
	
	state = MULTIPLAYER_GAME_STATE;
	connection_splashed = false;
}

void connection_failed_frame(void)
{
	add_text_renderends( "Failed to connect", 17, 100, 120, 6.0f );
	
	// "Go back" button and text
	add_renderend(UI_TEXTURE,   0, 66, 112, 17,   100, 270, 672, 102);
	add_text_renderends( "Go back", 7, 320, 290, 6.0f );
	// Cursor
	int hl_sx = (frame_number/20) % 2? 0: 15;
	add_renderend(UI_TEXTURE, hl_sx, 49, 14, 15, 100, 276, 84, 90);
	
	// Show the error description
	add_text_renderends( conn_menu_err_desc, conn_menu_err_strlen, 100, 500, 4.0f );
	
	if (ESC_PRESSED || ENTER_PRESSED || SPACE_PRESSED) {
		state = CONNECTION_MENU_STATE;
	}
}
