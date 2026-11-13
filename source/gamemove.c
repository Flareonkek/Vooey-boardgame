
// Check if a square belongs to the player designated by argflag, arggeo0, and arggeo1
unsigned char isplayers(
	unsigned char square, unsigned char argflag, unsigned char arggeo0, unsigned char arggeo1,
	unsigned char *board_cfs, unsigned char *board_geo_c0s, unsigned char *board_geo_c1s )
{
	if( board_cfs[square] == argflag &&
		board_geo_c0s[square] == arggeo0 &&
		board_geo_c1s[square] == arggeo1 )
	{
		return 1;
	}
	return 0;
}

unsigned char n_surrounding(unsigned char square, unsigned char *field)
{
	// Takes a board-array 'field', which should have 1s and 0s, and tells you
	// how many 1s are in the 8 squares surrounding argument 'square'
	
	unsigned char check_west = (square % 10 != 0);
	unsigned char check_east = (square % 10 != 9);
	unsigned char check_north = (square > 9);
	unsigned char check_south = (square < 90);
	
	int n = 0;
	
	if (check_north)
	{
		if (field[square-10])
			n++;
		if (check_west)
			if (field[square-1-10])
				n++;
		if (check_east)
			if (field[square+1-10])
				n++;
	}
	if (check_south)
	{
		if (field[square+10])
			n++;
		if (check_west)
			if (field[square-1+10])
				n++;
		if (check_east)
			if (field[square+1+10])
				n++;
	}
	if (check_west)
		if (field[square-1])
			n++;
	if (check_east)
		if (field[square+1])
			n++;
	
	return n;
}

unsigned char is_border_square(unsigned char square)
{
	if (square % 10 == 0)
		return 1;
	if (square % 10 == 9)
		return 1;
	if (square < 10)
		return 1;
	if (square >= 90)
		return 1;
	return 0;
}

unsigned char floodfnc(unsigned char *field, unsigned char *flood, unsigned char start_sq)
{
	// Let field be a board of 0s with 1s for walls that stop the flood
	// Let flood be a board of 0s with 1s that expand as much as they can,
	// recursively. This function returns 1 if the flood hit a board border,
	// otherwise it returns 0, and in that case, will have set *flood to the
	// entire flooded area.
	
	// Don't do more work on squares we already flooded
	if (flood[start_sq])
		return 0;
	
	// Don't mess with squares blocked by field
	if (field[start_sq])
		return 0;
		
	//printf("FLOOD sq %i\n", start_sq);
	
	if (is_border_square(start_sq))
		return 1;
	
	if (1)
	{
		flood[start_sq] = 1;
		int n = 0;
		n += floodfnc(field, flood, start_sq-1);
		n += floodfnc(field, flood, start_sq+10);
		n += floodfnc(field, flood, start_sq-10);
		n += floodfnc(field, flood, start_sq+1);
		if (n)
			return 1;
	}
	
	
}

// Applies a move (by player with argflag etc. onto square) to array trio (board_cfs, board_geo_c0s, and board_geo_c1s)
char gamemove(
	unsigned char square, unsigned char argflag, unsigned char arggeo0, unsigned char arggeo1,
	unsigned char *board_cfs, unsigned char *board_geo_c0s, unsigned char *board_geo_c1s )
{
	// Square must be on the board
	if (square >= 100)
		return 1;
	
	// Square cannot already be occupied
	if (board_cfs[square] != UNOCCUPIED_SQUARE)
		return 1;
	
	// If we made it here, we have a valid move, and at the very least we will occupy this square
	board_cfs[square] = argflag;
	board_geo_c0s[square] = arggeo0;
	board_geo_c1s[square] = arggeo1;
	
	// Also, check if this move completes an enclosure, and if so, fill the enclosed area
	
	/*
	For an enclosure to have been made, there must be atleast two adjacent squares already owned by the player.
	Those being as they are, we must then check the non player-owned adjacents,
	And if we find that an attempt to flow out from those meets only more player-owned squares, we fill in that area.
	But if we meed a board-border, then the area is not enclosed. But if we meet a square owned by another player, we treat
	that the same as unoccupied, as an opponents square within an enclosed area gets taken over by the enclosing player.
	*/
	
	unsigned char field[100];
	
	for (unsigned char i = 0; i < 100; i++)
	{
		field[i] = isplayers(i, argflag, arggeo0, arggeo1, board_cfs, board_geo_c0s, board_geo_c1s);
	}
	
	int n_player_owned_neighbors = n_surrounding(square, field);
	
	//printf("n_player_owned_neighbors: %i\n", n_player_owned_neighbors);
	
	if (n_player_owned_neighbors < 2)
		return 0;
	
	// Test flooding out from the four squares in cardinal-directions from argument square (JUST THE WEST ONE FOR NOW!!!)
	
	if (square % 10 != 0) // (If we're not on the western border of the board)
	{
		if (field[square-1] == 0) // If the square west of arg 'square' isn't aleady player-occupied
		{
			unsigned char flood[100];
			memset(flood, 0, 100);
			if (!floodfnc(field, flood, square-1)) { // Start the flood in the square west of arg 'square'
				printf("western floodfnc() returned 0\n");
				for (int i = 0; i < 100; i++)
				{
					if (flood[i])
					{
						board_cfs[i] = argflag;
						board_geo_c0s[i] = arggeo0;
						board_geo_c1s[i] = arggeo1;
					}
				}
			}
			
		}
	}
	if (square % 10 != 9) // (If we're not on the eastern border of the board)
	{
		if (field[square+1] == 0) // If the square east of arg 'square' isn't aleady player-occupied
		{
			unsigned char flood[100];
			memset(flood, 0, 100);
			if (!floodfnc(field, flood, square+1)) { // Start the flood in the square east of arg 'square'
				printf("eastern floodfnc() returned 0\n");
				for (int i = 0; i < 100; i++)
				{
					if (flood[i])
					{
						board_cfs[i] = argflag;
						board_geo_c0s[i] = arggeo0;
						board_geo_c1s[i] = arggeo1;
					}
				}
			}
			
		}
	}
	if (square >= 10) // (If we're not on the northern border of the board)
	{
		if (field[square-10] == 0) // If the square north of arg 'square' isn't aleady player-occupied
		{
			unsigned char flood[100];
			memset(flood, 0, 100);
			if (!floodfnc(field, flood, square-10)) { // Start the flood in the square north of arg 'square'
				printf("northern floodfnc() returned 0\n");
				for (int i = 0; i < 100; i++)
				{
					if (flood[i])
					{
						board_cfs[i] = argflag;
						board_geo_c0s[i] = arggeo0;
						board_geo_c1s[i] = arggeo1;
					}
				}
			}
			
		}
	}
	if (square < 90) // (If we're not on the southern border of the board)
	{
		if (field[square+10] == 0) // If the square south of arg 'square' isn't aleady player-occupied
		{
			unsigned char flood[100];
			memset(flood, 0, 100);
			if (!floodfnc(field, flood, square+10)) { // Start the flood in the square south of arg 'square'
				printf("southern floodfnc() returned 0\n");
				for (int i = 0; i < 100; i++)
				{
					if (flood[i])
					{
						board_cfs[i] = argflag;
						board_geo_c0s[i] = arggeo0;
						board_geo_c1s[i] = arggeo1;
					}
				}
			}
			
		}
	}
	return 0;
}



















