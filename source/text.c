
const int ui_char_w = 7;
const int ui_char_h = 11;


int ascii_sprite_widths[95] = // The width of my hand-drawn sprite for each ASCII code
{
//  !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7  8  9  :  ;
	1, 3, 5, 5, 5, 5, 1, 3, 3, 5, 5, 2, 3, 1, 5, 5, 3, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3,
//  <  =  >  ?  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 6, 5, 5,
//  W  X  Y  Z  [  \  ]  ^  _  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r
	6, 5, 5, 5, 3, 5, 3, 5, 5, 3, 5, 5, 5, 5, 5, 5, 5, 5, 2, 5, 5, 2, 7, 5, 5, 5, 5, 5,
//  s  t  u  v  w  x  y  z  {  |  }  ~  [out-of-range]
	5, 4, 5, 5, 6, 5, 5, 5, 4, 1, 4, 5, 7
};

void set_ascii_sprite_src(char arg_ascii, int* src_x, int* src_y, int* src_w) {
	if (arg_ascii < '!' || arg_ascii > '~') {
		// Invalid ASCII [?] symbol
		(*src_x) = 96;
		(*src_y) = 36;
		(*src_w) = 7;
		return;
	}
	(*src_w) = ascii_sprite_widths[arg_ascii-'!'];
	if (arg_ascii <= ';') {
		(*src_y) = 0;
		(*src_x) = (arg_ascii-'!')*8;
		return;
	}
	if (arg_ascii <= 'V') {
		(*src_y) = 12;
		(*src_x) = (arg_ascii-'<')*8;
		return;
	}
	if (arg_ascii <= 'r') {
		(*src_y) = 24;
		(*src_x) = (arg_ascii-'W')*8;
		return;
	}
	(*src_y) = 36;
	(*src_x) = (arg_ascii-'s')*8;
	return;
}

int add_text_renderends(char* text, int text_len, int x, int y, float scale_factor)
{
	float cx = (float)x;
	int cy = y;
	for (int i = 0; i < text_len; i++) {
		char thischar = text[i];
		if (thischar == ' ') {
			cx += (scale_factor*ui_char_w);
			continue;
		}
		if (thischar == '\0')
			break;
		int sx, sy, sw;
		set_ascii_sprite_src(thischar, &sx, &sy, &sw);
		add_renderend(UI_TEXTURE, sx, sy, ui_char_w, ui_char_h, (int)cx, cy, (int)(scale_factor*ui_char_w), (int)(scale_factor*ui_char_h));
		cx += (scale_factor*(sw+1));
	}
	return (int)cx;
}

#define ESC_PRESSED (keycode_p & keycode_bits[47])
#define F1_PRESSED (keycode_p & keycode_bits[48])
#define F2_PRESSED (keycode_p & keycode_bits[49])
#define F3_PRESSED (keycode_p & keycode_bits[50])
#define F4_PRESSED (keycode_p & keycode_bits[51])

#define BACKSPACE_PRESSED (keycode_p & keycode_bits[52])
#define TAB_PRESSED (keycode_p & keycode_bits[53])
#define ENTER_PRESSED (keycode_p & keycode_bits[54])
#define SHIFT_PRESSED (keycode_p & keycode_bits[55])
#define CTRL_PRESSED (keycode_p & keycode_bits[56])
#define ALT_PRESSED (keycode_p & keycode_bits[57])
#define SPACE_PRESSED (keycode_p & keycode_bits[58])
#define UP_ARROW_PRESSED (keycode_p & keycode_bits[59])
#define LEFT_ARROW_PRESSED (keycode_p & keycode_bits[60])
#define DOWN_ARROW_PRESSED (keycode_p & keycode_bits[61])
#define RIGHT_ARROW_PRESSED (keycode_p & keycode_bits[62])
#define PRINTSCREEN_PRESSED (keycode_p & keycode_bits[63])

#define BACKSPACE_HELD (keycode_i & keycode_bits[52])
#define SHIFT_HELD (keycode_i & keycode_bits[55])
#define CTRL_HELD (keycode_i & keycode_bits[56])
#define ALT_HELD (keycode_i & keycode_bits[57])
#define UP_ARROW_HELD (keycode_i & keycode_bits[59])
#define LEFT_ARROW_HELD (keycode_i & keycode_bits[60])
#define DOWN_ARROW_HELD (keycode_i & keycode_bits[61])
#define RIGHT_ARROW_HELD (keycode_i & keycode_bits[62])

#define W_PRESSED (keycode_p & keycode_bits[32])
#define A_PRESSED (keycode_p & keycode_bits[10])
#define S_PRESSED (keycode_p & keycode_bits[28])
#define D_PRESSED (keycode_p & keycode_bits[13])

char keycode_miniscules[47] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'`', '-', '=', '[', ']', '\\', ';', '\'', ',', '.', '/'
};

char keycode_majuscules[47] = {
	')', '!', '@', '#', '$', '%', '^', '&', '*', '(',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'~', '_', '+', '{', '}', '|', ':', '"', '<', '>', '?'
};

int bckspc_held_frames = 0;
int l_arro_held_frames = 0;
int r_arro_held_frames = 0;

// Uses keyboard input to type stuff in text field (basic version where you cannot select previous characters)
// Cursor is separate from len in this version, as it always assumes the cursor is at the end of the field
void type_to_field20_basic(char field[20], int* text_len, int* text_cursor)
{
	if (BACKSPACE_HELD)
		bckspc_held_frames++;
	else
		bckspc_held_frames = 0;
	if ((BACKSPACE_PRESSED || bckspc_held_frames > 13) && (*text_len) > 0) {
		if ((*text_cursor) == (*text_len)) {
			(*text_len)--;
			(*text_cursor)--;
		}
	}
	
	if ((*text_len) >= 20)
		return;
	
	// The first 47 keycodes can produce typable characters
	for (int i = 0; i < 47; i++) {
		if (keycode_p & keycode_bits[i]) {
			field[(*text_len)] = (SHIFT_HELD? keycode_majuscules[i]: keycode_miniscules[i]);
			(*text_len)++;
			(*text_cursor)++;
		}
	}
	// The only remaining typable to check for is a space
	if (SPACE_PRESSED) {
		field[(*text_len)] = ' ';
		(*text_len)++;
		(*text_cursor)++;
	}
}

// Uses keyboard input to type stuff in text field
void type_to_field(char* field, int* text_len, int* text_cursor, int field_size)
{
	if (LEFT_ARROW_HELD)
		l_arro_held_frames++;
	else
		l_arro_held_frames = 0;
		
	if (RIGHT_ARROW_HELD)
		r_arro_held_frames++;
	else
		r_arro_held_frames = 0;
	
	if (BACKSPACE_HELD)
		bckspc_held_frames++;
	else
		bckspc_held_frames = 0;
	
	if ((LEFT_ARROW_PRESSED || l_arro_held_frames > 13) && (*text_cursor) > 0)
		(*text_cursor)--;
	if ((RIGHT_ARROW_PRESSED || r_arro_held_frames > 13) && (*text_cursor) < (*text_len))
		(*text_cursor)++;
	
	if ((BACKSPACE_PRESSED || bckspc_held_frames > 13) && (*text_cursor) > 0) {
		for (int i = (*text_cursor); i < (*text_len); i++)
			field[i-1] = field[i];
		(*text_len)--;
		(*text_cursor)--;
	}
	
	if ((*text_len) >= field_size)
		return;
	
	// The first 47 keycodes can produce typable characters
	for (int i = 0; i < 47; i++) {
		if (keycode_p & keycode_bits[i] && (*text_len) < field_size) {
			for (int i = (*text_len); i > (*text_cursor); i--)
				field[i] = field[i-1];
			field[(*text_cursor)] = (SHIFT_HELD? keycode_majuscules[i]: keycode_miniscules[i]);
			(*text_len)++;
			(*text_cursor)++;
		}
	}
	// The only remaining typable to check for is a space
	if (SPACE_PRESSED && (*text_len) < field_size) {
		for (int i = (*text_len); i > (*text_cursor); i--)
			field[i] = field[i-1];
		field[(*text_cursor)] = ' ';
		(*text_len)++;
		(*text_cursor)++;
	}
}
