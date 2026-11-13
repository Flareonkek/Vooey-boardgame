
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXTURES_LEN 69
#define TFPP "flag_images/"
char* texture_filepaths[TEXTURES_LEN] = {
	TFPP "apocryphal.png",
	TFPP "geoflags.png",
	TFPP "pepe.png",
	TFPP "averi.png",
	TFPP "burned_furs.png",
	TFPP "soyjak.png",
	
	TFPP "christian.png", // (index 6)
	TFPP "chud.png",
	TFPP "averichud.png",
	TFPP "gigachad.png",
	TFPP "bob.png",
	TFPP "pokeball.png",
	
	TFPP "pikachu.png", // (index 12)
	TFPP "tails.png",
	TFPP "ferret.png",
	TFPP "sylveon.png",
	TFPP "leafeon.png",
	TFPP "glaceon.png",
	
	TFPP "espeon.png", // (index 18)
	TFPP "umbreon.png",
	TFPP "jolteon.png",
	TFPP "vaporeon.png",
	TFPP "flareon.png",
	TFPP "eevee.png",
	
	TFPP "sylveon_shiny.png", // (index 24)
	TFPP "plier.png",
	TFPP "flag_ceaera.png",
	TFPP "new_averi.png",
	TFPP "flag_tux.png",
	TFPP "flag_macos.png",
	
	TFPP "flag_windows.png", // (index 30)
	TFPP "clover.png",
	TFPP "hazel.png",
	TFPP "cotton.png",
	TFPP "briar.png",
	TFPP "blueberry.png",
	
	TFPP "flag_shadow.png", // (index 36)
	TFPP "flag_knuckles.png",
	TFPP "flag_sonic.png",
	TFPP "flag_silver.png",
	TFPP "flag_beau.png",
	TFPP "flag_rover.png",
	
	TFPP "purefur.png", // (index 42)
	TFPP "pure_fur_ball.png",
	TFPP "burned_fur_ball.png",
	TFPP "kirby.png",
	TFPP "wolfjak_flag.png",
	TFPP "flag_kedamono.png",
	
	TFPP "flag_ame.png", // (index 48)
	TFPP "dewy.png",
	TFPP "cheetah.png",
	TFPP "Blinkyflag.png",
	TFPP "daneclaw.png",
	TFPP "flag_braiman.png",
	
	TFPP "dragon_green.png", // (index 54)
	TFPP "dragon_red.png",
	TFPP "elka.png",
	TFPP "flag_sigrid.png",
	TFPP "flag_eina.png",
	TFPP "flag_eli.png",
	
	TFPP "flag_seibold.png", // (index 60)
	TFPP "sherman.png",
	TFPP "kola.png",
	TFPP "gamilia.png",
	TFPP "choco.png",
	TFPP "moth.png",
	
	TFPP "aggretsuko.png", // (index 66)
	TFPP "avali.png",
	
	"ui.png"
};
#undef TFPP

#define UI_TEXTURE 68

// DATATYPE FOR THE PROGRAM TO ASK FOR THINGS TO BE RENDERED/DISPLAYED
struct render_instruction
{
	int texture_index,
	src_x, src_y, src_w, src_h,
	dest_x, dest_y, dest_w, dest_h;
};

// GLOBAL ARRAY OF struct render_instructionS TO BE DISPLAYED (IN THE ORDER THEY'RE IN IN THE ARRAY)
struct render_instruction renderends[500];
const int renderends_len = 500; // The size of the array (and thus, the maximum number of render instructions that can be in effect at a time)
int renderends_index = 0; // Which index of the array is currently in use. add_renderend() adds to this index, and when we si_draw(), it draws this many things.

void add_renderend(int texture_index, int src_x, int src_y, int src_w, int src_h, int dest_x, int dest_y, int dest_w, int dest_h)
{
	if (renderends_index >= renderends_len) {
		printf("ERROR: add_renderend() attempted with full renderends array\n");
		exit(1);
	}
	renderends[ renderends_index ] = (struct render_instruction) {
		.texture_index=texture_index,
		.src_x=src_x,
		.src_y=src_y,
		.src_w=src_w,
		.src_h=src_h,
		.dest_x=dest_x,
		.dest_y=dest_y,
		.dest_w=dest_w,
		.dest_h=dest_h
	};
	renderends_index++;
}

// All core rendering will be done as if the display dimensions were 1920 by 1080.
// The actual display will also always be made to have the same 16:9 aspect ratio,
// but it will be scaled up or down as needed depending on the size of the window.
const int idea_w = 1920;
const int idea_h = 1080;
const float idea_aspect_ratio = (float)idea_w / (float)idea_h;
const float idea_aspect_ratio_inverse = (float)idea_h / (float)idea_w;

int disp_x = 0;
int disp_y = 0;
int disp_w = idea_w/2;
int disp_h = idea_h/2;
float disp_scaling = 0.5;

// RAYLIB is used to create a window, display things in the window, and play sounds, but only via the functions after this point in this very file
// (so that the source code could be rewritten to use a different library by changes only to this file)
#include "raylib.h"

static Texture game_textures[TEXTURES_LEN];

void si_initialize(void)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE); //Set before InitWindow

	InitWindow(disp_w, disp_h, "Vooey board game or something"); // The window starts off with dimensions 960 by 540
	
	SetExitKey(KEY_NULL); // Disable the default escape-key closing the window action
	
	for (int i = 0; i < TEXTURES_LEN; i++) {
		game_textures[i] = LoadTexture( texture_filepaths[i] );
	}
	
	SetTargetFPS(30);
}

bool si_window_close_order = false;

bool si_running(void) {
	return !(WindowShouldClose() || si_window_close_order);
}

void si_renderframe(void)
{
	// Check the window dimensions and set disp_x, disp_y, disp_w, and disp_h accordingly
	{
		int sw = GetScreenWidth();
		int sh = GetScreenHeight();
		if ( ((float)sw/(float)sh) < idea_aspect_ratio ) { // The window is less wide than idea dimensions
			// Then we use the width to scale display
			disp_w = sw;
			disp_h = disp_w * idea_aspect_ratio_inverse;
			disp_x = 0;
			disp_y = (sh - disp_h)/2;
			disp_scaling = (float)disp_w / (float)idea_w;
		} else {
			// Then we use the height to scale display
			disp_h = sh;
			disp_w = disp_h * idea_aspect_ratio;
			disp_x = (sw - disp_w)/2;
			disp_y = 0;
			disp_scaling = (float)disp_h / (float)idea_h;
		}
	}
	
	
	
	BeginDrawing();
	
	ClearBackground(BLACK); // Fill the window black
	
	DrawRectangle(disp_x, disp_y, disp_w, disp_h, WHITE); // Fill the display area white
	
	for (int i = 0; i < renderends_index; i++) {
		struct render_instruction this = renderends[i];
		DrawTexturePro(
			game_textures[this.texture_index], // texture: What texture to render
			(Rectangle) {this.src_x,  this.src_y,  this.src_w,  this.src_h }, // source: what part of it to render
			(Rectangle) {this.dest_x*disp_scaling+disp_x, this.dest_y*disp_scaling+disp_y, this.dest_w*disp_scaling, this.dest_h*disp_scaling}, // dest: where to render it onscreen
			(Vector2) {0, 0}, 0.0f, WHITE // SNCA
		);
		//printf("disp x: %i, y: %i, w: %i, h: %i, scaling %f\n", disp_x, disp_y, disp_w, disp_h, disp_scaling);
	}
	
	EndDrawing();
	
	renderends_index = 0; // Done doing this frame's renderends, so we reset this in readiness for the next frame
}

void si_playsound(int sound_index)
{
	
}

void si_deinitialize(void)
{
	for (int i = 0; i < TEXTURES_LEN; i++)
		UnloadTexture(game_textures[i]);
		
	CloseWindow();
}

const unsigned long long keycode_bits[64] = {
	0b0000000000000000000000000000000000000000000000000000000000000001, // 0
	0b0000000000000000000000000000000000000000000000000000000000000010, // 1
	0b0000000000000000000000000000000000000000000000000000000000000100, // 2
	0b0000000000000000000000000000000000000000000000000000000000001000, // 3
	0b0000000000000000000000000000000000000000000000000000000000010000, // 4
	0b0000000000000000000000000000000000000000000000000000000000100000, // 5
	0b0000000000000000000000000000000000000000000000000000000001000000, // 6
	0b0000000000000000000000000000000000000000000000000000000010000000, // 7
	0b0000000000000000000000000000000000000000000000000000000100000000, // 8
	0b0000000000000000000000000000000000000000000000000000001000000000, // 9
	0b0000000000000000000000000000000000000000000000000000010000000000, // A
	0b0000000000000000000000000000000000000000000000000000100000000000, // B
	0b0000000000000000000000000000000000000000000000000001000000000000, // C
	0b0000000000000000000000000000000000000000000000000010000000000000, // D
	0b0000000000000000000000000000000000000000000000000100000000000000, // E
	0b0000000000000000000000000000000000000000000000001000000000000000, // F
	0b0000000000000000000000000000000000000000000000010000000000000000, // G
	0b0000000000000000000000000000000000000000000000100000000000000000, // H
	0b0000000000000000000000000000000000000000000001000000000000000000, // I
	0b0000000000000000000000000000000000000000000010000000000000000000, // J
	0b0000000000000000000000000000000000000000000100000000000000000000, // K
	0b0000000000000000000000000000000000000000001000000000000000000000, // L
	0b0000000000000000000000000000000000000000010000000000000000000000, // M
	0b0000000000000000000000000000000000000000100000000000000000000000, // N
	0b0000000000000000000000000000000000000001000000000000000000000000, // O
	0b0000000000000000000000000000000000000010000000000000000000000000, // P
	0b0000000000000000000000000000000000000100000000000000000000000000, // Q
	0b0000000000000000000000000000000000001000000000000000000000000000, // R
	0b0000000000000000000000000000000000010000000000000000000000000000, // S
	0b0000000000000000000000000000000000100000000000000000000000000000, // T
	0b0000000000000000000000000000000001000000000000000000000000000000, // U
	0b0000000000000000000000000000000010000000000000000000000000000000, // V
	0b0000000000000000000000000000000100000000000000000000000000000000, // W
	0b0000000000000000000000000000001000000000000000000000000000000000, // X
	0b0000000000000000000000000000010000000000000000000000000000000000, // Y
	0b0000000000000000000000000000100000000000000000000000000000000000, // Z
	0b0000000000000000000000000001000000000000000000000000000000000000, // `
	0b0000000000000000000000000010000000000000000000000000000000000000, // -
	0b0000000000000000000000000100000000000000000000000000000000000000, // =
	0b0000000000000000000000001000000000000000000000000000000000000000, // [
	0b0000000000000000000000010000000000000000000000000000000000000000, // ]
	0b0000000000000000000000100000000000000000000000000000000000000000, // '\'
	0b0000000000000000000001000000000000000000000000000000000000000000, // ;
	0b0000000000000000000010000000000000000000000000000000000000000000, // '
	0b0000000000000000000100000000000000000000000000000000000000000000, // ,
	0b0000000000000000001000000000000000000000000000000000000000000000, // .
	0b0000000000000000010000000000000000000000000000000000000000000000, // /
	0b0000000000000000100000000000000000000000000000000000000000000000, // Escape
	0b0000000000000001000000000000000000000000000000000000000000000000, // F1
	0b0000000000000010000000000000000000000000000000000000000000000000, // F2
	0b0000000000000100000000000000000000000000000000000000000000000000, // F3
	0b0000000000001000000000000000000000000000000000000000000000000000, // F4
	0b0000000000010000000000000000000000000000000000000000000000000000, // Backspace
	0b0000000000100000000000000000000000000000000000000000000000000000, // Tab
	0b0000000001000000000000000000000000000000000000000000000000000000, // Enter
	0b0000000010000000000000000000000000000000000000000000000000000000, // Shift
	0b0000000100000000000000000000000000000000000000000000000000000000, // Control
	0b0000001000000000000000000000000000000000000000000000000000000000, // Alt
	0b0000010000000000000000000000000000000000000000000000000000000000, // Space
	0b0000100000000000000000000000000000000000000000000000000000000000, // Up-arrow
	0b0001000000000000000000000000000000000000000000000000000000000000, // Left-arrow
	0b0010000000000000000000000000000000000000000000000000000000000000, // Down-arrow
	0b0100000000000000000000000000000000000000000000000000000000000000, // Right-arrow
	0b1000000000000000000000000000000000000000000000000000000000000000, // Print-screen
};

int si_raylib_key_reps[64] = {
	KEY_ZERO,
	KEY_ONE,
	KEY_TWO,
	KEY_THREE,
	KEY_FOUR,
	KEY_FIVE,
	KEY_SIX,
	KEY_SEVEN,
	KEY_EIGHT,
	KEY_NINE,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_GRAVE,
	KEY_MINUS,
	KEY_EQUAL,
	KEY_LEFT_BRACKET,
	KEY_RIGHT_BRACKET,
	KEY_BACKSLASH,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_ESCAPE,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_ENTER,
	KEY_LEFT_SHIFT,
	KEY_LEFT_CONTROL,
	KEY_LEFT_ALT,
	KEY_SPACE,
	KEY_UP,
	KEY_LEFT,
	KEY_DOWN,
	KEY_RIGHT,
	KEY_PRINT_SCREEN
};

void si_check_keys(unsigned long long *keycode) {
	(*keycode) = 0;
	for (int i = 0; i < 64; i++) {
		if ( IsKeyDown(si_raylib_key_reps[i]) ) {
			(*keycode) += keycode_bits[i];
		}
	}
}
