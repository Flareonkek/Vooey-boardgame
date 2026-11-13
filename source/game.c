

//   ##########################################
//   ###                                    ###
//   ###   COMMON GAME DATA AND FUNCTIONS   ###
//   ###                                    ###
//   ##########################################

/* The game board is a 10x10 board, represented in three parts, _cfs, _c0s, and _c1s.

The _cfs part represents the index of what flag occupies each square, or 255 for an UNOCCUPIED squares
The other parts _c0s and _c1s are there in case the flag occupying a square is a geoflag,
in which case the two extra bytes are needed to specify which country it is.
*/
unsigned char gameboard_cfs[100];
unsigned char gameboard_c0s[100];
unsigned char gameboard_c1s[100];
int gameboard_data_status = 0;

#define UNOCCUPIED_SQUARE 255

/*
The client keeps a list of all the players in the server so you can see who you're playing with.
When recieving list-data from the server, you must keep track of where to put it as follows:

int playerlist_data_status represents the index of the player whose data is being read in

That playerdata's .data_status represents the byte of his data that should be read,
(0~19 for chars in his name, 20 for his flag, 21 and 22 for geoflag, and 23 for ready-state)
*/

struct playerdata {
	// Basic data
	char *name;
	unsigned char flag;
	unsigned char geoflag0;
	unsigned char geoflag1;
	// (Ready to start game?)
	unsigned char ready;
	
	// How far along in the process are we of getting this player's data:
	int data_status;
	// Any value less than 23 represents a byte of their basic data that I'm waiting to hear.
};

#define MAX_PLAYERS 10

struct playerdata playerlist[MAX_PLAYERS];
int playerlist_data_status = 0; // How far along in the process are we of getting the list-data

int playercount = 0;

unsigned char my_cursor = 44; // Start the cursor in the middle of the board. This represents a gameboard square's index.

void enact_cursor(void)
{
	if (A_PRESSED || LEFT_ARROW_PRESSED)
	{
		if (my_cursor == 0) {
			my_cursor = 9;
		} else {
			my_cursor--;
			
			if (my_cursor%10 == 9)
				my_cursor += 10;
		}
	}
	if (D_PRESSED || RIGHT_ARROW_PRESSED)
	{
		my_cursor++;
		
		if (!(my_cursor%10))
			my_cursor -= 10;
	}
	if (W_PRESSED || UP_ARROW_PRESSED)
	{
		if (my_cursor < 10)
			my_cursor += 100;
		my_cursor -= 10;
	}
	if (S_PRESSED || DOWN_ARROW_PRESSED)
	{
		my_cursor += 10;
		if (my_cursor >= 100)
			my_cursor -= 100;
	}
	
	// Square width and height
	static const int sw = 120;
	static const int sh = 83;
	
	// Offset of game-board position
	static const int x_offset = 670;
	static const int y_offset = 200;
	
	//void add_renderend(int texture_index, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h)
	// Display highlight on the current selection on the grid of flags
	//add_renderend(0, 182, 18, 78, 54, ((player_flag%6)-1)*78+800, (player_flag-(player_flag%6)-6)*9+400, 234, 162);
	
	/*
	add_renderend(
		0, 183, 19, 76, 52, //182, 18, 78, 54, // texture_index and source frame
		(my_cursor%10)*sw+x_offset-sw, // dest_x
		(my_cursor/10)*sh+y_offset-sh, // dest_y
		sw*3, sh*3 // dest_w, dest_h
		);
	*/
	int hl_xo = (frame_number/10) % 2? 0: 26;
	
	add_renderend(
		0, 104+hl_xo, 18, 26, 18, // texture_index and source frame
		(my_cursor%10)*sw+x_offset, // dest_x
		(my_cursor/10)*sh+y_offset, // dest_y
		sw, sh // dest_w, dest_h
		);
}

void game_init(void)
{
	for (int i = 0; i < 100; i++)
	{
		gameboard_cfs[i] = UNOCCUPIED_SQUARE;
		gameboard_c0s[i] = 0;
		gameboard_c1s[i] = 0;
	}
	
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		// void *calloc(size_t nmemb, size_t size);
		playerlist[i].name = calloc(MAX_PLAYERS, sizeof(struct playerdata));
	}
}

void game_deinit()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		free(playerlist[i].name);
	}
}

void render_gameboard(void)
{
	// Square width and height
	static const int sw = 120;
	static const int sh = 83;
	
	// Offset of game-board position
	static const int x_offset = 670;
	static const int y_offset = 200;
	
	// The basic grid of empty squares
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			add_renderend( 0, // texture index
				0, 0, 26, 18, // src_x, _y, _w, _h
				j*sw+x_offset, // dest_x
				i*sh+y_offset, // dest_y
				sw, sh // dest_w, _h
				);
		}
	}
	
	// The flags on any occupied squares
	int gameboard_i = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (gameboard_cfs[gameboard_i] != UNOCCUPIED_SQUARE)
			{
				add_flag_renderend(
					gameboard_cfs[gameboard_i], // flag_index
					ucharpair_to_geoflag( // arg_geo
						gameboard_c0s[gameboard_i],
						gameboard_c1s[gameboard_i]),
					j*sw+x_offset, // dest_x
					i*sh+y_offset, // dest_y
					sw, sh // dest_w, dest_h
					);
			}
			gameboard_i++;
		}
	}
}

enum {
	STATE_UNKNOWN,
	STATE_LOBBY,
	STATE_GAME,
	STATE_POSTGAME
};
int gamestate = STATE_UNKNOWN;

//   ###########################################
//   ###                                     ###
//   ###   O F F L I N E   G A M E P L A Y   ###
//   ###                                     ###
//   ###########################################

#include "gamemove.c"

void singleplayer_game_frame(void)
{
	render_gameboard();
	enact_cursor();
	if (SPACE_PRESSED || ENTER_PRESSED)
		gamemove(
			my_cursor, player_flag, geoflag_c0, geoflag_c1,
			gameboard_cfs, gameboard_c0s, gameboard_c1s
		);
	if (ESC_PRESSED)
	{
		memset(gameboard_cfs, UNOCCUPIED_SQUARE, 100);
		memset(gameboard_c0s, 0, 100);
		memset(gameboard_c1s, 0, 100);
		state = MAIN_MENU_STATE;
	}
}

//   #########################################
//   ###                                   ###
//   ###   O N L I N E   G A M E P L A Y   ###
//   ###                                   ###
//   #########################################

/* Reminder about data we're going to send to the server
char player_name[20];
unsigned char player_flag;
unsigned char geoflag_c0;
unsigned char geoflag_c1;
*/

// (>480 since there can be 20 players in the server, and for a full playerlist, we'd get 24 bytes per player)
// (It's important because with a non-blocking socket, extra bytes can get lost, so I wanna handle all of them!)
#define MAX_DATA_SIZE 500
char buf[MAX_DATA_SIZE];

enum {
	DATAMODE_AWAIT,
	DATAMODE_EXPECT_PLAYERCOUNT,
	DATAMODE_EXPECT_PLAYERDATA,
	DATAMODE_EXPECT_BOARDDATA
};
int datamode = DATAMODE_AWAIT;

unsigned char me_ready = 0; // (For STATE_LOBBY) Whether I'm ready to start game

unsigned char my_turn = 0; // (For STATE_GAME) Whether it's my turn
int this_turn_frames = 0; // For how many frames it has been my/whoever's turn

void multiplayer_game_frame(void)
{
	// Store any data we recieved to buf and set bytes_recvd -------------------------------------------------
	
	int bytes_recvd = recv(sockfd, buf, MAX_DATA_SIZE, 0);
	if (bytes_recvd < 0)
	{
		// Non-blocking socket will return -1 and set errno to EAGAIN or EWOULDBLOCK if no data arrived
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			perror("recv"); // (Anything other than those may be a problem, and I should know about it)
		
		bytes_recvd = 0;
	}
	else if (bytes_recvd == 0)
	{
		char *serverclosedstr = "Recieved message of 0 bytes from server, indicating connection closed";
		printf("%s\n", serverclosedstr);
		state = CONNECTION_FAILED_STATE;
		for (int i = 0; i < 69; i++)
			conn_menu_err_desc[i] = serverclosedstr[i];
		conn_menu_err_strlen = 69;
	}
	
	// Handle the recieved data ------------------------------------------------------------------------------
	
	for (int i = 0; i < bytes_recvd; i++)
	{
		if (datamode == DATAMODE_AWAIT)
		{
			/*
				SERVER-TO-CLIENT communication protocol:
				
				'A': You have joined as a player
				'B': You can't join, server's full
				
				'C': Prepare to recieve player-list data from me...
				
				'D': We are in the lobby (waiting to start) so you may send me your ready-or-not status
				'E': We are in-game
				'F': We are in post-game (Screen congratulating whoever won)
	
				'G': It's your turn to move
				'H': It's someone else's turn to move
				
				'I': Prepare to recieve game-board data from me...
			*/
			
			if (buf[i] == 'A')
			{
				printf("recieved 'A', sending my player-data...");
				
				int bytes_sent = 0;
				while (bytes_sent < 20)
				{
					int this_send = send(sockfd, player_name+bytes_sent, 20-bytes_sent, 0);
					if (this_send < 0)
						perror("send");
					else
						bytes_sent += this_send;
				}
				while (bytes_sent < 21)
				{
					int this_send = send(sockfd, &player_flag, 1, 0);
					if (this_send < 0)
						perror("send");
					else
						bytes_sent += this_send;
				}
				while (bytes_sent < 22)
				{
					unsigned char msg = (player_flag == FLAG_GEO ? geoflag_c0 : 0);
					int this_send = send(sockfd, &msg, 1, 0);
					if (this_send < 0)
						perror("send");
					else
						bytes_sent += this_send;
				}
				while (bytes_sent < 23)
				{
					unsigned char msg = (player_flag == FLAG_GEO ? geoflag_c1 : 0);
					int this_send = send(sockfd, &msg, 1, 0);
					if (this_send < 0)
						perror("send");
					else
						bytes_sent += this_send;
				}
				printf(" done\n");
				// (That's that, I've sent all 23 bytes of player-data)
			}
			else if (buf[i] == 'B')
			{
				char *serverfullstr = "Cannot join, server is full";
				printf("%s\n", serverfullstr);
				state = CONNECTION_FAILED_STATE;
				for (int i = 0; i < 27; i++)
					conn_menu_err_desc[i] = serverfullstr[i];
				conn_menu_err_strlen = 27;
				
				#ifdef _WIN32
				WSACleanup();
				#else
				close(sockfd);
				#endif
			}
			else if (buf[i] == 'C')
			{
				printf("recieved 'C', expecting player-count...");
				datamode = DATAMODE_EXPECT_PLAYERCOUNT;
			}
			else if (buf[i] == 'D')
			{
				printf("recieved 'D', setting gamestate to STATE_LOBBY\n");
				gamestate = STATE_LOBBY;
			}
			else if (buf[i] == 'E')
			{
				printf("recieved 'E', setting gamestate to STATE_GAME\n");
				gamestate = STATE_GAME;
			}
			else if (buf[i] == 'F')
			{
				printf("recieved 'F', setting gamestate to STATE_POSTGAME\n");
				gamestate = STATE_POSTGAME;
			}
			else if (buf[i] == 'G')
			{
				printf("recieved 'G', IT IS MY TURN\n");
				my_turn = 1;
				this_turn_frames = 0;
			}
			else if (buf[i] == 'H')
			{
				printf("recieved 'H', IT IS NOT MY TURN\n");
				my_turn = 0;
				this_turn_frames = 0;
			}
			else if (buf[i] == 'I')
			{
				printf("recieved 'I', expecting board-data...\n");
				gameboard_data_status = 0;
				datamode = DATAMODE_EXPECT_BOARDDATA;
			}
		}
		else if (datamode == DATAMODE_EXPECT_PLAYERCOUNT)
		{
			playercount = (int)buf[i]; // Note the number of players
			printf(" got %i\n", playercount);
			playerlist_data_status = 0; // Note that we haven't processed any of the list data yet
			if (playercount > 0)
			{
				for (int j = 0; j < MAX_PLAYERS; j++)
					playerlist[j].data_status = 0; // Note that we haven't processed any of each player's data yet
				datamode = DATAMODE_EXPECT_PLAYERDATA;
			}
			else
			{
				datamode = DATAMODE_AWAIT;
			}
		}
		else if (datamode == DATAMODE_EXPECT_PLAYERDATA)
		{
			// Process this byte of data according to playerlist_data_status and the appropriate players's .data_status
			//printf("Expecting playerdata, playerlist_data_status is %i\n", playerlist_data_status);
			
			int player_i = playerlist_data_status; // Which player in the list are we describing?
			int player_byte = playerlist[ player_i ].data_status; // Which byte of their data?
			
			//printf("playerlist[%i].data_status is %i\n", player_i, player_byte);
			
			if (player_byte < 20)
			{
				//printf("setting .name byte %i to %i (%c)\n", player_byte, buf[i], buf[i]);
				playerlist[player_i].name[player_byte] = buf[i];
				playerlist[player_i].data_status++;
			}
			else if (player_byte == 20)
			{
				//printf("setting .flag byte to %i\n", buf[i]);
				playerlist[player_i].flag = buf[i];
				playerlist[player_i].data_status++;
			}
			else if (player_byte == 21)
			{
				//printf("setting .geoflag0 byte to %i\n", buf[i]);
				playerlist[player_i].geoflag0 = buf[i];
				playerlist[player_i].data_status++;
			}
			else if (player_byte == 22)
			{
				//printf("setting .geoflag1 byte to %i\n", buf[i]);
				playerlist[player_i].geoflag1 = buf[i];
				playerlist[player_i].data_status++;
			}
			else if (player_byte == 23)
			{
				//printf("setting .ready byte to %i\n", buf[i]);
				playerlist[player_i].ready = buf[i];
				// That would finish that player's data, so we'll increment playerlist_data_status
				playerlist_data_status++;
				printf("Got player-data: name %s, flag %i, geoflag %i and %i, ready: %i\n",
					playerlist[player_i].name, playerlist[player_i].flag, playerlist[player_i].geoflag0,
					playerlist[player_i].geoflag1, playerlist[player_i].ready);
			}
			
			// If that finished us the last player in the list, return to DATAMODE_AWAIT
			if (playerlist_data_status >= playercount)
				datamode = DATAMODE_AWAIT;
		}
		else if (datamode == DATAMODE_EXPECT_BOARDDATA)
		{
			// gameboard_data_status starts at 0, and goes to 99 telling us the flag on each square,
			// then 100~199 telling us the first geo-char, and 200~299 the second geo_char on each square
			
			if (gameboard_data_status < 100)
			{
				gameboard_cfs[gameboard_data_status] = buf[i];
				gameboard_data_status++;
			}
			else if (gameboard_data_status < 200)
			{
				gameboard_c0s[gameboard_data_status-100] = buf[i];
				gameboard_data_status++;
			}
			else
			{
				gameboard_c1s[gameboard_data_status-200] = buf[i];
				gameboard_data_status++;
				if (gameboard_data_status >= 300)
				{
					datamode = DATAMODE_AWAIT;
					printf("Finished recieving board-data\n");
				}
			}
		}
	}
	
	// Do normal frame stuff ---------------------------------------------------------------------------------
	
	if (gamestate == STATE_LOBBY) // -------------------------------------------------------------------------
	{
		// Render server's name and number of occupants out of ten, at the top of the screen
		
		int mx = add_text_renderends(conn_sstr, conn_sstr_len, 50, 10, 6.0f);
		mx = add_text_renderends(" (", 2, mx, 10, 6.0f);
		if (playercount < 10)
		{
			char digit = '0' + playercount;
			mx = add_text_renderends(&digit, 1, mx, 10, 6.0f);
		}
		else
			mx = add_text_renderends("10", 2, mx, 10, 6.0f);
		add_text_renderends("/10)", 4, mx, 10, 6.0f);
		
		// Below that, render the list of players in the server
		
		for (int i = 0; i < playercount; i++)
		{
			// Render each player's flag
			add_flag_renderend(
				playerlist[i].flag,
				ucharpair_to_geoflag(playerlist[i].geoflag0, playerlist[i].geoflag1),
				300, 100+100*i, 26*4, 18*4
				);
			// And their name
			int nx = add_text_renderends(playerlist[i].name, 20, 425, 100+100*i, 5.0f);
			
			if (playercount > 1)
			{
				// And their ready-status, by a hand signal + text
				add_renderend(
					0, // apocryphal.png
					playerlist[i].ready? 26 : 0, // src_x,
					36, 26, 18, // _y, _w, _h
					250, 100+100*i, 26*4, 18*3
					);
				if (playerlist[i].ready)
					add_text_renderends("(Ready to start)", 16, nx+20, 100+100*i, 3.0f);
				else
					add_text_renderends("(Not ready)", 11, nx+20, 100+100*i, 3.0f);
			}
		}
		if (playercount < 2)
		{
			me_ready = 0;
			return;
		}
		if (SPACE_PRESSED)
		{
			if (me_ready)
				me_ready = 0;
			else
				me_ready = 1;
				
			if (send(sockfd, &me_ready, 1, 0) < 1)
				perror("send");
			else
				printf("client: sent %i to server\n", me_ready);
		}
	}
	else if (gamestate == STATE_GAME) // ---------------------------------------------------------------------
	{
		render_gameboard();
		
		// Render a smaller player-list on the left side of the screen
		for (int i = 0; i < playercount; i++)
		{
			add_flag_renderend(
				playerlist[i].flag, //flag_ind
				ucharpair_to_geoflag( //arg geo
					playerlist[i].geoflag0,
					playerlist[i].geoflag1),
				26, 18+54*i, 26*2, 18*2 );//dest coords
			
			add_text_renderends(
				playerlist[i].name, 20,
				100, 18+54*i, 4.0f );
		}
		
		if (my_turn)
		{
			// Display "YOUR TURN - Xs / 60s" (X being this_turn_frames)
			int mx = add_text_renderends("YOUR TURN - ", 12, 680, 75, 5.0f);
			if (this_turn_frames > 300)
			{
				char tensdigit = '0'+(this_turn_frames/300);
				mx = add_text_renderends(&tensdigit, 1, mx, 75, 5.0f);
			}
			char onesdigit = '0'+((this_turn_frames%300)/30);
			mx = add_text_renderends(&onesdigit, 1, mx, 75, 5.0f);
			add_text_renderends("s / 60s", 7, mx, 75, 5.0f);
			
			// Let me highlight a square
			enact_cursor(); // (Changes unsigned char my_cursor)
			
			// Let me submit a move on the highlighted square by pressing space or enter
			if (SPACE_PRESSED || ENTER_PRESSED)
			{
				if (send(sockfd, &my_cursor, 1, 0) < 1)
					perror("send");
				else
					printf("client: sent %i to server\n", my_cursor);
			}
			
		}
		else // (It is someone else's turn)
		{
			// Display "Waiting for other player's turn - Xs" (X being this_turn_frames)
			int mx = add_text_renderends("Waiting for other player's turn - ", 34, 680, 75, 5.0f);
			if (this_turn_frames > 300)
			{
				char tensdigit = '0'+(this_turn_frames/300);
				mx = add_text_renderends(&tensdigit, 1, mx, 75, 5.0f);
			}
			char onesdigit = '0'+((this_turn_frames%300)/30);
			mx = add_text_renderends(&onesdigit, 1, mx, 75, 5.0f);
			add_text_renderends("s", 1, mx, 75, 5.0f);
		}
		this_turn_frames++;
	}
	else if (gamestate == STATE_POSTGAME) // -----------------------------------------------------------------
	{
		render_gameboard();
		
		// Display "Game finished"
		add_text_renderends("Game finished", 13, 680, 75, 5.0f);
		
		// TODO make this display the scores n shiet (calculate them)
		
		int hiscore = 0;
		int winner_ind = 0;
		
		// Render a smaller player-list on the left side of the screen
		for (int i = 0; i < playercount; i++)
		{
			// Render the player's flag
			add_flag_renderend(
				playerlist[i].flag, //flag_ind
				ucharpair_to_geoflag( //arg geo
					playerlist[i].geoflag0,
					playerlist[i].geoflag1),
				26, 18+54*i, 26*2, 18*2 );//dest coords
			
			/* (Dont render their name)
			add_text_renderends(
				playerlist[i].name, 20,
				100, 18+54*i, 4.0f );
			*/
			
			// Calculate this player's score
			int his_score = 0;
			for (int j = 0; j < 100; j++)
			{
				if (gameboard_cfs[j] == playerlist[i].flag &&
					gameboard_c0s[j] == playerlist[i].geoflag0 &&
					gameboard_c1s[j] == playerlist[i].geoflag1 )
				{
					his_score++;
				}
			}
			// Render the player's score next to their flag
			if (his_score==100)
			{
				add_text_renderends(
					"100", 3,
					100, 18+54*i, 4.0f );
			} else {
				int mx = 100;
				if (his_score > 9)
				{
					char tensdigit = '0'+(his_score/10);
					mx = add_text_renderends(
							&tensdigit, 1,
							mx, 18+54*i, 4.0f );
				}
				char onesdigit = '0'+(his_score%10);
				add_text_renderends(
						&onesdigit, 1,
						mx, 18+54*i, 4.0f );
			}
			// Keep track of the highest score
			if (his_score > hiscore)
			{
				hiscore = his_score;
				winner_ind = i;
			}
		}
		
		// Render an award next to the winner's list-spot
		
		//void add_renderend(int texture_index, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h)
		
		add_renderend(
			UI_TEXTURE, 0, 100, 95, 125, // Award texture and source
			175, 18+54*winner_ind, 50, 50 // dest_x, _y, _w, and _h
			);
		
	}
	else // (gamestate must be STATE_UNKNOWN) ----------------------------------------------------------------
	{
		add_text_renderends("Waiting for game-state data...", 30, 300, 400, 5.0f);
	}
}
