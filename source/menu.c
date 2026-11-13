
#define FLAG_GEO 1

void add_flag_renderend(int flag_index, int arg_geo, int dest_x, int dest_y, int dest_w, int dest_h)
{
	int sx = 0;
	int sy = 0;
	int sw = 26;
	int sh = 18;
	if (flag_index == 0) {
		// nophono is 234 pixels to the right
		sx = 234;
	}
	if (flag_index == FLAG_GEO) {
		// geoflags depend on arg_geo
		set_geoflag_source(arg_geo, &sx, &sy, &sw, &sh);
	}
	if (flag_index == 43) {
		// pure_fur_ball is 75x52
		sw = 75;
		sh = 52;
	}
	if (flag_index == 44) {
		// burned_fur_ball is 38x26
		sw = 38;
		sh = 26;
	}
	add_renderend(flag_index, sx, sy, sw, sh, dest_x, dest_y, dest_w, dest_h);
}

// #############################################################################################################################
// ###                                                                                                                       ###
// ###                    M A I N           M E N U           D A T A          &          F U N C T I O N                    ###
// ###                                                                                                                       ###
// #############################################################################################################################

const int main_menu_items_len = 6;
int main_menu_cursor_item = 0; // Starts hovering by PLAY-SINGLEPLAYER

const int main_menu_str_len = 17;
char* main_menu_item_texts[6] = {
	"Play singleplayer",
	"Play multiplayer ",
	"Name:            ",
	"Flag:            ",
	"Light/Dark mode  ",
	"Quit             ",
};

const int player_name_max_len = 19;
const int player_name_chars = 20;
char player_name[20];
int player_name_len = 0;
int player_name_cursor = 0;

unsigned char player_flag = 0; // Starts off as Nophono flag

bool main_menu_in_setting = false; // Goes TRUE when you interact with your player name or flag, back to FALSE when done.

void main_menu_frame(void)
{	
	// RENDER THE BUTTONS AND THEIR TEXT -----------------------------------------------------------
	
	for (int i = 0; i < main_menu_items_len; i++) {
		int tby = 100+(i*150);
		//                        [---source----] [-----dest-------]
		add_renderend(UI_TEXTURE, 0, 66, 112, 17, 100, tby, 672, 102); // Button
		add_text_renderends( main_menu_item_texts[i], main_menu_str_len, 200, tby+20, 6.0f );
		if (i == 2) {
			add_renderend(UI_TEXTURE,   5, 66, 102, 17,   692, tby, 608, 102); // Middle part of a button, extending the button
			add_renderend(UI_TEXTURE,   107, 66, 5, 17,   1300, tby, 30, 102); // Right half of a button, concluding the button
		}
	}
	
	// RENDER THE CURSOR ---------------------------------------------------------------------------
	
	int hl_sx = (frame_number/20) % 2? 0: 15;
	if (main_menu_in_setting)
		hl_sx += 30;
	int tby = 106+(main_menu_cursor_item*150);
	//                        [-----source-----] [-----dest-----]
	add_renderend(UI_TEXTURE, hl_sx, 49, 14, 15, 100, tby, 84, 90);
	
	// RENDER THE PLAYER DATA ----------------------------------------------------------------------
	
	if (!player_name_len && !(main_menu_cursor_item==2&&main_menu_in_setting)) {
		add_text_renderends( "(Nameroll)", 10, 400, 420, 6.0f );
	} else {
		// Render player_name
		add_text_renderends( player_name, player_name_len, 400, 420, 6.0f );
		int c_offset = add_text_renderends( player_name, player_name_cursor, 400, 420, 6.0f );
		if (main_menu_cursor_item==2 && main_menu_in_setting && (frame_number/10) % 3) {
			add_text_renderends("_", 1, c_offset, 420, 6.0f);
		}
	}
	
	add_flag_renderend(player_flag, geoflag, 375, 117+450, 26*4, 18*4);
	
	// RESPOND TO KEYBOARD INPUT --------------------------------------------------------------------------------------------
	
	if (main_menu_in_setting)
	{
		if (main_menu_cursor_item == 2) // We're editing the player NAME data ----------------------
		{
			if (! (ESC_PRESSED || ENTER_PRESSED) )
				type_to_field(player_name, &player_name_len, &player_name_cursor, 20);
		}
		else if (main_menu_cursor_item == 3) // We're selecting the player FLAG --------------------
		{
			// Display the grid of flags to the right of the menu
			for (int i = 0; i < 68; i++) {
				int dx = (i % 6) * 78 + 800;
				int dy = (i - (i%6) ) * 9 + 400;
				add_flag_renderend(i, geoflag, dx, dy, 26*3, 18*3);
			}
			// Display highlight on the current selection on the grid of flags
			add_renderend(0, 182, 18, 78, 54, ((player_flag%6)-1)*78+800, (player_flag-(player_flag%6)-6)*9+400, 234, 162);
			// Let WASD/Arrow keys change the flag selection
			if (LEFT_ARROW_PRESSED || A_PRESSED) {
				if (player_flag == 0)
					player_flag = 67;
				else
					player_flag--;
			}
			if (RIGHT_ARROW_PRESSED || D_PRESSED) {
				player_flag++;
				if (player_flag > 67)
					player_flag = 0;
			}
			if (UP_ARROW_PRESSED || W_PRESSED) {
				if (player_flag < 6)
					player_flag += 66;
				else
					player_flag -= 6;
				if (player_flag > 67)
					player_flag -= 6;
			}
			if (DOWN_ARROW_PRESSED || S_PRESSED) {
				player_flag += 6;
				if (player_flag > 67)
					player_flag = player_flag % 6;
			}
			if (SPACE_PRESSED) // (Space can get you out if you're in flag selection)
				main_menu_in_setting = false;
		}
		// (Escape or Enter gets you out in either case)
		if (ESC_PRESSED || ENTER_PRESSED) {
			main_menu_in_setting = false;
		}
	}
	else // We're not actively modifying any setting, but just in the menu --------------------------------------------------
	{
		// Move cursor by Up/Down arrows or W/S
		if (DOWN_ARROW_PRESSED || S_PRESSED) {
			main_menu_cursor_item++;
			if (main_menu_cursor_item >= main_menu_items_len)
				main_menu_cursor_item = 0;
		} else if (UP_ARROW_PRESSED || W_PRESSED) {
			main_menu_cursor_item--;
			if (main_menu_cursor_item < 0)
				main_menu_cursor_item = main_menu_items_len-1;
		}
		// Do button actions by Space or Enter
		if (SPACE_PRESSED || ENTER_PRESSED) {
			if (main_menu_cursor_item == 0)
				state = SINGLEPLAYER_GAME_STATE;
			
			if (main_menu_cursor_item == 1)
				state = CONNECTION_MENU_STATE;
			
			if (main_menu_cursor_item == 2 || main_menu_cursor_item == 3)
				main_menu_in_setting = true;
			
			if (main_menu_cursor_item == 4)
				darkmode = !darkmode;
			
			if (main_menu_cursor_item == 5)
				si_window_close_order = true;
		}
	}
}

// ############################################################################################################################
// ###                                                                                                                      ###
// ###                    C O N N E C T I O N        M E N U        D A T A       &       F U N C T I O N                   ###
// ###                                                                                                                      ###
// ############################################################################################################################

const int conn_menu_items_len = 4;
int conn_menu_cursor_item = 0;

const int conn_menu_str_len = 22;
char* conn_menu_item_texts[4] = {
	"Connect               ",
	"Server address:       ",
	"Server's port number: ",
	"Go back               ",
};

const int conn_sstr_max_len = 40; // (Big enough to accomodate a full IPv6 address)
char conn_sstr[41]; // (That, plus one for \0)
int conn_sstr_len = 0;
int conn_sstr_cursor = 0;

const int conn_pstr_max_len = 5; // Big enough to accomodate the highest valid port number (65535)
char conn_pstr[6]; // Big enough for the longest possible nul-terminated string "65535\0"
int conn_pstr_len = 0;
int conn_pstr_cursor = 0;

bool conn_menu_in_setting = false;

void connection_menu_frame(void)
{
	// RENDER THE BUTTONS AND THEIR TEXT -----------------------------------------------------------
	
	for (int i = 0; i < conn_menu_items_len; i++) {
		int tby = 100+(i*150);
		//                         [---source----]   [------dest-------]
		add_renderend(UI_TEXTURE,   0, 66, 112, 17,   50, tby, 672, 102); // Button
		if (i == 1 || i == 2) {
			add_renderend(UI_TEXTURE,   5, 66, 102, 17,   692, tby, 1108, 102); // Middle part of a button, extending the button
			add_renderend(UI_TEXTURE,   107, 66, 5, 17,   1800, tby, 30, 102); // Right half of a button, concluding the button
		}
		add_text_renderends( conn_menu_item_texts[i], conn_menu_str_len, 150, tby+20, 6.0f );
	}
	
	// RENDER THE CURSOR ---------------------------------------------------------------------------
	
	int hl_sx = (frame_number/20) % 2? 0: 15;
	if (conn_menu_in_setting)
		hl_sx += 30;
	int tby = 106+(conn_menu_cursor_item*150);
	//                        [-----source-----] [-----dest-----]
	add_renderend(UI_TEXTURE, hl_sx, 49, 14, 15, 50, tby, 84, 90);
	
	// RENDER THE ENTERED SERVER DATA --------------------------------------------------------------
	
	add_text_renderends( conn_sstr, conn_sstr_len, 700, 270, 6.0f );
	int c_offset = add_text_renderends( conn_sstr, conn_sstr_cursor, 700, 270, 6.0f );
	if (conn_menu_cursor_item==1 && conn_menu_in_setting && (frame_number/10) % 3) {
		add_text_renderends("_", 1, c_offset, 270, 6.0f);
	}
	
	add_text_renderends( conn_pstr, conn_pstr_len, 900, 420, 6.0f );
	c_offset = add_text_renderends( conn_pstr, conn_pstr_cursor, 900, 420, 6.0f );
	if (conn_menu_cursor_item==2 && conn_menu_in_setting && (frame_number/10) % 3) {
		add_text_renderends("_", 1, c_offset, 420, 6.0f);
	}
	
	// RESPOND TO KEYBOARD INPUT -------------------------------------------------------------------
	
	if (conn_menu_in_setting)
	{
		if (ESC_PRESSED || ENTER_PRESSED)
		{
			conn_menu_in_setting = false;
		}
		else if (conn_menu_cursor_item == 1) // We're editing the server address field -------------
		{
			type_to_field(conn_sstr, &conn_sstr_len, &conn_sstr_cursor, 40);
		}
		else if (conn_menu_cursor_item == 2) // We're editing the server's port # field ------------
		{
			if (keycode_p & 0b1000011111101111111111111111111111111111111111111111110000000000)
				return; // <- if any key is pressed that isnt an arrow key, backspace, or digit
			type_to_field(conn_pstr, &conn_pstr_len, &conn_pstr_cursor, 5);
		}
	}
	else
	{
		// Return to Main menu by Escape key
		if (ESC_PRESSED) {
			state = MAIN_MENU_STATE;
		}
		// Move cursor by Up/Down arrows or W/S
		if (DOWN_ARROW_PRESSED || S_PRESSED) {
			conn_menu_cursor_item++;
			if (conn_menu_cursor_item >= conn_menu_items_len)
				conn_menu_cursor_item = 0;
		} else if (UP_ARROW_PRESSED || W_PRESSED) {
			conn_menu_cursor_item--;
			if (conn_menu_cursor_item < 0)
				conn_menu_cursor_item = conn_menu_items_len-1;
		}
		// Do button actions by Space or Enter
		if (SPACE_PRESSED || ENTER_PRESSED) {
			if (conn_menu_cursor_item == 0) {
				state = CONNECTION_ATTEMPT_STATE;
			}
			
			if (conn_menu_cursor_item == 1 || conn_menu_cursor_item == 2)
				conn_menu_in_setting = true;
			
			if (conn_menu_cursor_item == 3)
				state = MAIN_MENU_STATE;
		}
	}
}
