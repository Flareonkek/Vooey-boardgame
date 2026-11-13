#include "system_implementation.c"

#define MAIN_MENU_STATE 0
#define CONNECTION_MENU_STATE 1
#define CONNECTION_ATTEMPT_STATE 2
#define CONNECTION_FAILED_STATE 3
#define SINGLEPLAYER_GAME_STATE 4
#define MULTIPLAYER_GAME_STATE 5

int state = MAIN_MENU_STATE;

unsigned int frame_number = 0;

// Each bit in each of these represents a key (see keycode_bits in system_implementation.c)
unsigned long long keycode_i = 0ULL; // ON-bits for keys that are down
unsigned long long keycode_p = 0ULL; // ON-bits for keys that are pressed (down now but wasn't in the previous frame)
unsigned long long keycode_f = 0ULL; // ON-bits for keys that were down in previous frame (Used to determine keycode_p)

#include "geoflagging.c"
#include "text.c"
#include "menu.c"
#include "connection.c"
#include "game.c"

int main(void)
{
	printf("si_initialize()...\n");
    si_initialize();
    
    printf("geo_init()...\n");
	geo_init();
	
    printf("game_init()...\n");
	game_init();
	
	// [JUST FOR CONVENIENCE] this puts in a convenient server string for quick testing:
	/*
	conn_sstr[0] = 'l';
	conn_sstr[1] = 'o';
	conn_sstr[2] = 'c';
	conn_sstr[3] = 'a';
	conn_sstr[4] = 'l';
	conn_sstr[5] = 'h';
	conn_sstr[6] = 'o';
	conn_sstr[7] = 's';
	conn_sstr[8] = 't';
	conn_sstr_len = 9;
	conn_sstr_cursor = 9;
	*/
	// [/JUST FOR CONVENIENCE]
	
	// Put in the default Port # string
	conn_pstr[0] = '4';
	conn_pstr[1] = '7';
	conn_pstr[2] = '3';
	conn_pstr[3] = '2';
	conn_pstr[4] = '1';
	conn_pstr_len = 5;
	conn_pstr_cursor = 5;
	
    
	printf("main loop begins...\n");
    while(si_running()) {
		si_check_keys(&keycode_i);
		keycode_p = keycode_i & ~keycode_f;
		switch(state) {
			case MAIN_MENU_STATE:
				main_menu_frame();
				break;
			case CONNECTION_MENU_STATE:
				connection_menu_frame();
				break;
			case CONNECTION_ATTEMPT_STATE:
				connection_attempt_frame();	
				break;
			case CONNECTION_FAILED_STATE:
				connection_failed_frame();	
				break;	
			case SINGLEPLAYER_GAME_STATE:
				singleplayer_game_frame();
				break;
			case MULTIPLAYER_GAME_STATE:
				multiplayer_game_frame();
				break;
		}
		si_renderframe();
		frame_number++;
		keycode_f = keycode_i;
	}
	
	printf("game_deinit()...\n");
	game_deinit();
	
	printf("si_deinitialize()...\n");
	si_deinitialize();
}
