
//   #############################
//   ###                       ###
//   ###   G A M E   D A T A   ###
//   ###                       ###
//   #############################

enum {
	STATE_LOBBY,
	STATE_GAME,
	STATE_POSTGAME
};

int state = STATE_LOBBY;

// 10 x 10 board of flag-chars (and geoflag-chars)
unsigned char board_cfs[100];
unsigned char board_geo_c0s[100];
unsigned char board_geo_c1s[100];

#define UNOCCUPIED_SQUARE 255

time_t last_activity_time;
time_t postgame_started_time;

struct player {
	// Basic data
	char *name;
	unsigned char flag;
	unsigned char geoflag0;
	unsigned char geoflag1;
	// (Ready to start game?)
	unsigned char ready;
	
	// Socket / File-descriptor
	int fd;
	
	// How to interpret data from this player if I recieve any...
	int data_status;
	/* ...meaning:
	Any value less than 23 represents a byte of their basic data that I'm waiting to hear.
	*/
};

#define MAX_PLAYERS 10

struct player players[MAX_PLAYERS];
int n_players = 0;

int turn_playerind = 0; // Index of the player whose turn it is
time_t this_turn_start_time;

//   ###############################
//   ###                         ###
//   ###   AUXILIARY FUNCTIONS   ###
//   ###                         ###
//   ###############################

void send_char(int fd, unsigned char c)
{
	int bytes_sent = send(fd, &c, 1, 0);
	if (bytes_sent != 1)
		printf("warning from send_char: send returned bytes_sent of %i\n", bytes_sent);
}

void send_n_chars(int fd, char *c, int n)
{
	// Reminder about send():
	// ssize_t send(int sockfd, const void buf[.len], size_t len, int flags);
	int bytes_sent = 0;
	while (bytes_sent < n)
	{
		int this_send = send(fd, c+bytes_sent, n-bytes_sent, 0);
		bytes_sent += this_send;
	}
}

void send_everyone_playerlist(void)
{
	for (int i = 0; i < n_players; i++)
	{
		// Firstly, send them 'C' telling them to prepare for player-list data from me
		//printf("sending to %i: 'C', ", players[i].fd);
		send_char(players[i].fd, 'C');
		
		// Tell them how many players will be listed so they know how much data to expect
		//printf("(unsigned char)%i, ", (unsigned char)n_players);
		send_char( players[i].fd, (unsigned char)n_players );
		
		// Then send them every player's data
		for (int j = 0; j < n_players; j++)
		{
			struct player subject = players[j];
			//printf("24-byte playerdata%s", j == n_players-1?"\n":", ");
			send_n_chars( players[i].fd, subject.name, 20 );
			send_char( players[i].fd, subject.flag );
			send_char( players[i].fd, subject.geoflag0 );
			send_char( players[i].fd, subject.geoflag1 );
			send_char( players[i].fd, subject.ready );
		}
	}
}

void send_everyone_boarddata(void)
{
	for (int i = 0; i < n_players; i++)
	{
		// Firstly send them 'I' telling them to prepare for game-board data from me
		printf("sending to %i: 'I', ", players[i].fd);
		send_char(players[i].fd, 'I');
		
		// Then send the three hundred bytes they're expecting
		printf("sending 300-byte board-data\n");
		for (int j = 0; j < 100; j++)
		{
			send_char(players[i].fd, board_cfs[j]);
		}
		for (int j = 0; j < 100; j++)
		{
			send_char(players[i].fd, board_geo_c0s[j]);
		}
		for (int j = 0; j < 100; j++)
		{
			send_char(players[i].fd, board_geo_c1s[j]);
		}
		printf("finished sending board-data\n");
	}
}

void server_game_turn(void)
{
	this_turn_start_time = time(NULL);
	
	send_everyone_boarddata();
	
	for (int i = 0; i < n_players; i++)
	{
		// Send 'G' / 'H' ("Your turn started" / "Someone else's turn started")
		if (i == turn_playerind)
		{
			send_char(players[i].fd, 'G');
			printf("server: sent 'G' to socket %i\n", players[i].fd);
		}
		else
		{
			send_char(players[i].fd, 'H');
			printf("server: sent 'H' to socket %i\n", players[i].fd);
		}
	}
}

void server_game_nextturn(void)
{
	turn_playerind++;
	if (turn_playerind >= n_players)
		turn_playerind = 0;
	server_game_turn();
}

//   ###############################################
//   ###                                         ###
//   ###   FUNCTIONS CALLED FROM server_main.c   ###
//   ###                                         ###
//   ###############################################

// Runs once when the server starts
void server_init(void)
{
	printf("server_init()\n");
	
	// Clear out the board arrays
	memset(&board_cfs, UNOCCUPIED_SQUARE, 100);
	memset(&board_geo_c0s, 0, 100);
	memset(&board_geo_c1s, 0, 100);
	
	last_activity_time = time(NULL);
}

/*
	SERVER-TO-CLIENT communication protocol:
	
	'A': You have joined as a player
	'B': You can't join, server's full
	
	'C': Prepare to recieve player-list data from me...
	
	'D': We are in the lobby (waiting to start)
	'E': We are in-game
	'F': We are in post-game (Screen congratulating whoever won)
	
	'G': It's your turn to move
	'H': It's someone else's turn to move
	
	'I': Prepare to recieve game-board data from me...
	
*/

// Runs whenever something happens, but also every 2.5 seconds if nothing happens
void server_tick(void)
{
	printf("server_tick()\n");
	
	// In LOBBY, if we got no activity for 5s with everyone ready, we'll start the game
	if (state == STATE_LOBBY &&
		time(NULL)-last_activity_time >= 5 &&
		n_players > 1 )
	{
		// (Check that everyone is ready)
		for (int i = 0; i < n_players; i++)
		{
			if (!players[i].ready)
				return;
		}
		
		// If you reached this point, start the game!
		printf("STARTING GAME\n");
		state = STATE_GAME;
		for (int i = 0; i < n_players; i++)
		{
			send_char(players[i].fd, 'E');
			printf("server: sent 'E' to socket %i\n", players[i].fd);
		}
		
		// Let the first-joined player have the first turn
		turn_playerind = 0;
		server_game_turn();
	}
	
	// In GAME, if a player hasn't made their move in 60s, we'll skip their turn
	if (state == STATE_GAME &&
		time(NULL)-this_turn_start_time > 60 )
	{
		server_game_nextturn();
	}
	
	if (state == STATE_POSTGAME)
		printf("time(NULL) %ld, postgame_started_time %ld\n", time(NULL), postgame_started_time);
	
	// In POSTGAME, after 5 seconds, we'll return to LOBBY
	if (state == STATE_POSTGAME &&
		time(NULL)-postgame_started_time >= 5)
	{
		printf("returning to STATE_LOBBY\n");
		state = STATE_LOBBY;
		for (int i = 0; i < n_players; i++)
		{
			send_char(players[i].fd, 'D');
			printf("server: sent 'D' to socket %i\n", players[i].fd);
		}
		server_init(); // Sets the server board-data back to empty board
	}
}

void server_user_connected(int fd)
{
	printf("server_user_connected(): fd %i\n", fd);
	last_activity_time = time(NULL);
	
	if (n_players >= MAX_PLAYERS)
	{
		// Tell the newly-joined to go pound sand
		printf("server: sending 'B' to socket %i\n", fd);
		send_char(fd, 'B');
	}
	else
	{
		// Tell the newly-joined that they've joined successfully
		printf("server: sending 'A' to socket %i\n", fd);
		send_char(fd, 'A');
		// Add them to the players' array
		players[n_players].fd = fd; // Set the new player's socket
		players[n_players].data_status = 0;
		players[n_players].ready = 0;
		players[n_players].name = malloc(sizeof(char)*20);
		
		// (Temporary dummy data that will get replaced)
		players[n_players].name[0] = '?';
		for (int i = 1; i < 20; i++)
			players[n_players].name[i] = '\0';
		players[n_players].flag = 0;
		
		n_players++;
	}
}

void server_user_disconnected(int fd)
{
	printf("server_user_disconnected(): fd %i\n", fd);
	last_activity_time = time(NULL);
	
	// Find the quitter's player-array index
	for (int i = 0; i < n_players; i++)
	{
		if (players[i].fd == fd)
		{
			char redo_turn_flag = 0;
			
			// If in-game, and the quitter left during his own turn, set a flag
			if (state == STATE_GAME && turn_playerind == i)
			{
				redo_turn_flag = 1;
				turn_playerind--;
			}
			
			// Remove the quitter from the player-array
			for (int j = 0; j < 20; j++)
				players[i].name[j] = players[n_players-1].name[j];
			players[i].flag =        players[n_players-1].flag;
			players[i].geoflag0 =    players[n_players-1].geoflag0;
			players[i].geoflag1 =    players[n_players-1].geoflag1;
			players[i].ready =       players[n_players-1].ready;
			players[i].fd =          players[n_players-1].fd;
			players[i].data_status = players[n_players-1].data_status;
			
			n_players--;
			
			// Send everyone the updated playerlist
			send_everyone_playerlist();
			
			printf("redo_turn_flag is %i\n", redo_turn_flag);
			
			if (n_players == 1)
			{
				// If only one player is left, reset his ready-status to 0
				players[0].ready = 0;
				// And if in-game, declare him the winner of that round
				if (state == STATE_GAME)
				{
					state = STATE_POSTGAME;
					send_char(players[0].fd, 'F');
					postgame_started_time = time(NULL);
				}
			}
			else if (redo_turn_flag)
			{
				/* Since the quitter-removal will have placed another player into the
				 * array-spot formerly held by the quitter, we don't need to change
				 * turn_playerind, but rather just resend everyone the turn-start-char:
				*/
				server_game_turn();
			}
			
			break;
		}
	}
}

#include "gamemove.c"

void server_got_data(char buf[256], int nbytes, int fd)
{
	buf[nbytes] = '\0';
	printf("server_got_data() from fd %i: %s\n", fd, buf);
	last_activity_time = time(NULL);
	
	// Determine which player this is
	
	int player_i;
	for (int i = 0; i < n_players; i++)
	{
		if (players[i].fd == fd)
		{
			player_i = i;
			break;
		}
	}
	printf("(That's players[%i], named %s)\n", player_i, players[player_i].name);
	
	// Handle the data according to this player's data_status
	
	int bytes_processed = 0;
	
	while (bytes_processed < nbytes)
	{
		// ############################## DATA RECIEVED FROM NEWLY-JOINED PLAYERS ##############################
		if (players[player_i].data_status < 20)
		{
			players[player_i].name[players[player_i].data_status] = buf[bytes_processed];
			//printf("players[player_i] player's name[%i] set to %i\n", players[player_i].data_status, buf[bytes_processed]);
			players[player_i].data_status++;
			bytes_processed++;
		}
		else if (players[player_i].data_status == 20)
		{
			players[player_i].flag = buf[bytes_processed];
			//printf("players[player_i] player's flag set to %i\n", players[player_i].flag);
			players[player_i].data_status++;
			bytes_processed++;
		}
		else if (players[player_i].data_status == 21)
		{
			players[player_i].geoflag0 = buf[bytes_processed];
			//printf("players[player_i] player's geoflag0 set to %i\n", players[player_i].geoflag0);
			players[player_i].data_status++;
			bytes_processed++;
		}
		else if (players[player_i].data_status == 22)
		{
			players[player_i].geoflag1 = buf[bytes_processed];
			//printf("players[player_i] player's geoflag1 set to %i\n", players[player_i].geoflag1);
			players[player_i].data_status++;
			bytes_processed++;
			
			printf("players[%i]'s data all set (flag %i)\n", player_i, players[player_i].flag);
			
			// Since we've now gotten all the newly-joined player's info, time to share the updated player-list
			send_everyone_playerlist();
			
			// And also let the player know what state we're in
			if (state == STATE_LOBBY)
				send_char(fd, 'D');
			else if (state == STATE_GAME) {
				send_char(fd, 'E');
			} else { // (state == STATE_POSTGAME)
				send_char(fd, 'F');
			}
		}
		else // (.data_status > 22 ) The player has already introduced themself, so if we got data from them now...
		
		
		
		// ################################ DATA RECIEVED FROM ALREADY-JOINED PLAYERS ##################################
		{
			// ---------------------------------------------------------------------------------------------------------
			if (state == STATE_LOBBY) // ...They must be trying to update us on whether or not they're ready -----------
			{
				printf("server: got %i bytes of data from socket %i\n", nbytes-bytes_processed, fd);
				// Now we consider the last byte we got from this player, if it differs from their current
				// .ready status, then we must update our data and inform everyone of the change
				if (buf[nbytes-1] != players[player_i].ready)
				{
					players[player_i].ready = buf[nbytes-1];
					printf("player on socket %i is now %sready\n", fd, buf[nbytes-1]?"":"not ");
					send_everyone_playerlist();
				}
				// So we're done processing (having skipped any earlier data, which we would've changed again anyway)
				bytes_processed = nbytes;
			}
			
			// ---------------------------------------------------------------------------------------------------------
			else if (state == STATE_GAME) // ...They must be trying to tell us what move they want to make -------------
			{
				if (fd == players[turn_playerind].fd) // If it's their turn
				{
					printf("Got %i bytes of data from socket %i, during their turn\n", nbytes-bytes_processed, fd);
					
					// Let's just interpret the last byte as the move they want to make
					unsigned char their_move = buf[nbytes-1];
					
					if (their_move < 100)
					{
						printf("Running server_gamemove() for player's move to square %i\n", their_move);
						// GAME LOGIC mostly in HERE!!!
						if (gamemove(
							their_move, players[turn_playerind].flag,
							players[turn_playerind].geoflag0,
							players[turn_playerind].geoflag1,
							board_cfs, board_geo_c0s, board_geo_c1s)
							) // Returns 0 if they can make that move, 1 if invalid
						{
							return;
						}
						
						// Check if that filled up the last square on the board
						unsigned char all_occupied = 1;
						for (int i = 0; i < 100; i++)
						{
							if (board_cfs[i] == UNOCCUPIED_SQUARE) {
								all_occupied = 0;
								break;
							}
						}
						
						if (all_occupied) // If it did, end the game
						{
							printf("all squares occupied!\n");
							send_everyone_boarddata();
							// Tell everyone we're now in POSTGAME
							for (int i = 0; i < n_players; i++) {
								send_char(players[i].fd, 'F');
								players[i].ready = 0;
							}
							postgame_started_time = time(NULL);
							state = STATE_POSTGAME;
						}
						
						server_game_nextturn();
					}
					
				}
				else // (It's not this player's turn)
				{
					printf("Got %i bytes of data from socket %i, but it's not their turn\n", nbytes-bytes_processed, fd);
				}
				bytes_processed = nbytes; // (Just say we processed everything)
			}
			
			// ---------------------------------------------------------------------------------------------------------
			else // ...We must be in postgame, nobody should have anything to tell us ----------------------------------
			{
				printf("server: got %i bytes of unexpected data from socket %i\n", nbytes-bytes_processed, fd);
				bytes_processed = nbytes; // (Just say we processed everything)
			}
			// ---------------------------------------------------------------------------------------------------------
		}
	}
}
