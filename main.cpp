#include <cmath>
#include <math.h>
#include <vector>
#include <allegro.h>

using namespace std;

const int scr_w = 800;
const int scr_h = 600;
const int scl = 20;
const int width = 10;
const int height = 20;
const int left_bound = (scr_w -  width * scl) / 2;
const int right_bound = (scr_w + width * scl) / 2;
const int upper_bound = 4 * scl;
const int bottom_bound = upper_bound + scl * height;

volatile long counter = 0;
void increment() { counter++; }

int score = 0;
char scoreHolder[18];

int level = 0;
char levelHolder[3];

bool gameOver = false;
bool skipPiece = true;

enum shapes { I, J, L, O, S, T, Z };

const vector<vector<bool>> piece_I =
{
	{ 1, 1, 1, 1 },
	{ 0, 0, 0, 0 }
};
const vector<vector<bool>> piece_J =
{
	{ 1, 1, 1 },
	{ 0, 0, 1 }
};
const vector<vector<bool>> piece_L =
{
	{ 1, 1, 1 },
	{ 1, 0, 0 }
};
const vector<vector<bool>> piece_O =
{
	{ 1, 1 },
	{ 1, 1 }
};
const vector<vector<bool>> piece_S =
{
	{ 0, 1, 1 },
	{ 1, 1, 0 }
};
const vector<vector<bool>> piece_T =
{
	{ 1, 1, 1 },
	{ 0, 1, 0 }
};
const vector<vector<bool>> piece_Z =
{
	{ 1, 1, 0},
	{ 0, 1, 1}
};




/* General functions */
void square(BITMAP *source, int x, int y, int color, bool outline)
{
	if (outline)
	{
		rect(source, x, y, x + scl, y + scl, makecol(0, 0, 0));
		rectfill(source, x + 1, y + 1, x + scl-1, y + scl-1, color);
	}
	else
	{
		rectfill(source, x, y, x + scl, y + scl, color);
	}
}



/* Class GameArea */
class GameArea
{
public:
	// Functions
	GameArea();

	void draw(BITMAP *source);
	void setOccupation(vector<vector<bool>> piece, int color, int row, int col);
	void update();

	// Vars
	bool gameBoard[height][width];

private:
	
	void deleteRow(int row);
	void addScore(int combo);
	void clearLines();

	int colorBoard[height][width];
	int voidColor = makecol(225, 225, 255);
};

GameArea::GameArea()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			gameBoard[i][j] = false;
			colorBoard[i][j] = voidColor;
		}
	}
}

void GameArea::draw(BITMAP *source)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int x = left_bound + j * scl;
			int y = upper_bound + i * scl;

			

			if (!gameBoard[i][j])
			{
				if (j > 0 && j < width) x++;
				if (i > 0 && i < height) y++;
				square(source, x, y, voidColor, false);
			}
			else
			{
				square(source, x, y, colorBoard[i][j], true);
				
			}
		}
	}
}

void GameArea::setOccupation(vector<vector<bool>> piece, int color, int row, int col)
{
	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] == true)
			{
				if (row + i < height && col + j < width)
				{
					gameBoard[row + i][col + j] = true;
					colorBoard[row + i][col + j] = color;
				}
			}			
		}
	}
}

void GameArea::update()
{
	clearLines();
}

void GameArea::deleteRow(int row)
{
	for (int j = 0; j < width; j++)
	{
		gameBoard[row][j] = false;
	}
}

void GameArea::addScore(int combo)
{
	switch (combo)
	{
	case 1:
		score += 40 * (level + 1);
		break;
	case 2:
		score += 100 * (level + 1);
		break;
	case 3:
		score += 300 * (level + 1);
		break;
	case 4:
		score += 1200 * (level + 1);
		break;

	default:
		EXIT_FAILURE;
	}
}

void GameArea::clearLines()
{
	int highestRowDeleted;
	int lowestRowDeleted;
	int combo = 0;							// Counting number of rows cleared in single instance

	for (int i = 0; i < height; i++)
	{
		int occupied = 0;					// Counting number of occupied squares in a row
		for (int j = 0; j < width; j++)
		{
			if (gameBoard[i][j] == true)
			{
				occupied++;
			}
		}
		if (occupied == width)
		{
			if (combo == 0)
			{
				highestRowDeleted = i;

			}
			deleteRow(i);
			combo++;
		}
	}
	if (combo > 0)
	{
		lowestRowDeleted = highestRowDeleted + combo;

		for (int i = lowestRowDeleted - 1; i >= 0; i--)
		{
			for (int j = 0; j < width; j++)
			{
				if (gameBoard[i][j] == true)
				{
					int fall = lowestRowDeleted - i;
					if (fall > combo)
					{
						fall = combo;
					}

					gameBoard[i + fall][j] = true;
					colorBoard[i + fall][j] = colorBoard[i][j];
					gameBoard[i][j] = false;
					colorBoard[i][j] = voidColor;
				}
			}
		}

		SAMPLE *scoreSample = load_sample("Score.wav");
		play_sample(scoreSample, 20 + 5 * (combo - 1), 127, 1000 + 200 * (combo - 1), 0);
	}

	addScore(combo);
}




/* Class Shape */

class Shape
{

public:
	// Functions
	Shape(int type, bool activePiece, GameArea ga);

	void update(BITMAP *source, GameArea &ga);
	void draw(BITMAP *source, GameArea ga);
	void controls(GameArea ga);
	void setPosition(int x, int y) { this->x = x; this->y = y; }

	int getCol() { return col; }
	int getRow() { return row; }
	int getColor() { return color; }

	bool isActive() { return active; }

	vector<vector<bool>> getPiece() { return piece; }


	// Vars
	bool erase = false; // If true, erase shape next gameloop
	

private:
	// Vars
	vector<vector<bool>> piece;

	int row;
	int col;

	int x = scr_w / 2 - scl;
	int y = upper_bound;
	
	int dx = scl;
	int dy = 1;

	int rotation = 0;
	int color;			

	int stdbuff = 10;
	int lastMinBuffer = 2 * stdbuff; // Allows a few last seconds of movement before deactivating the piece
	int pos_x_buffer  = stdbuff; // Avoids too fast +x-movement
	int neg_x_buffer  = stdbuff; // Avoids too fast -x-movement
	int space_buffer  = stdbuff; // Avoids accidental double piece drop
		

	bool active			= true;	 // Active piece = falling piece
	bool repeated		= false; // Avoids repeated keypresses
	bool left_blocked   = false; // True piece is blocked by occupied square to the left
	bool right_blocked  = false; // True piece is blocked by occupied square to the right
	bool lastMinuteMove = false; // Allows a few last seconds of movement before deactivating the piece
	bool speedMove		= false; // Avoids too fast x-movement
	bool space_pressed  = false;

	bool anti_spacelock;
	bool anti_erase;


	// Functions
	void rotate(GameArea ga);
	void checkIfBlocked(GameArea ga);
	void addFalling(GameArea ga);
	void adjustToBoundaries();
	void antiKeyLock();
	void updateRowAndColumn();
	void floatPieceLeft();

	int getMinDist(GameArea ga);
	int x_left();
	int x_right();
	int y_bottom();
	int y_top();
	int bottom_row();
	int left_col();
	int right_col();

	bool illegalCollision(GameArea ga, vector<vector<bool>> piece);
	bool groundCollision(GameArea ga);
};

Shape::Shape(int type, bool activePiece, GameArea ga)
{
	switch (type)
	{
	case I:
		piece = piece_I;
		color = makecol(255, 0, 0);
		break;

	case J:
		piece = piece_J;
		color = makecol(255, 0, 255);
		break;

	case L:
		piece = piece_L;
		color = makecol(255, 255, 0);
		break;

	case O:
		piece = piece_O;
		color = makecol(0, 255, 255);
		break;

	case S:
		piece = piece_S;
		color = makecol(0, 0, 255);
		break;

	case T:
		piece = piece_T;
		color = makecol(255, 165, 0);
		break;

	case Z:
		piece = piece_Z;
		color = makecol(150, 255, 0);
		break;


	default:
		EXIT_FAILURE; // Invalid shape
		break;
	}

	active = activePiece;

	updateRowAndColumn();

	if (key[KEY_SPACE])
	{
		anti_spacelock = true;
	}
	else
	{
		anti_spacelock = false;
	}

	if (key[KEY_C])
	{
		anti_erase = true;
	}
	else
	{
		anti_erase = false;
	}


	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && ga.gameBoard[row + i][col + j])
			{
				gameOver = true;
			}
		}
	}



}

void Shape::update(BITMAP *source, GameArea &ga)
{
	updateRowAndColumn();

	floatPieceLeft();

	antiKeyLock();

	checkIfBlocked(ga);

	if (groundCollision(ga))
	{
		if (lastMinBuffer > 0)  // Last-minute-move mechanism
		{
			lastMinuteMove = true;
			lastMinBuffer--;

			if (lastMinBuffer <= 0 || space_pressed)
			{
				ga.setOccupation(piece, color, row, col);
				active = false;
				skipPiece = true;
			}
		}
	}
	else if (active)
	{
		addFalling(ga);
	}
	
	level = score / 1000;
	
	/*row = (y - upper_bound) / scl;						// Update row position
	col = (x - left_bound) / scl;						// Update col position

	controls(ga);										// Keyboard input handler

	if (space_pressed)
	{
		row = (y - upper_bound) / scl;						// Update row position
		col = (x - left_bound) / scl;						// Update col position

		ga.setOccupation(piece, color, row, col);
		active = false;
	}

	if (active)
	{
		left_blocked = false;
		right_blocked = false;

		for (int i = 0; i < piece.size(); i++)				// Check if piece is blocked on the sides
		{
			for (int j = 0; j < piece[0].size(); j++)
			{
				if (piece[i][j] == true)
				{
					if (row + i < height)
					{
						if (col + j - 1 >= 0)
						{
							if (ga.gameBoard[row + i][col + j - 1] == true)
							{
								left_blocked = true;
							}
						}
						if (col + j + 1 < width)
						{
							if (ga.gameBoard[row + i][col + j + 1] == true)
							{
								right_blocked = true;
							}
						}
					}
				}
			}
		}

		x_left = scr_w;
		x_right = 0;
		y_bottom = 0;
		y_top = scr_h;

		for (int i = 0; i < piece.size(); i++)
		{
			for (int j = 0; j < piece[0].size(); j++)
			{
				if (piece[i][j] == true)
				{
					if (x + j * scl < x_left)			// Leftmost x-coordinate of piece
					{
						x_left = x + j * scl;
					}
					if (x + (j + 1) * scl > x_right)    // Rightmost x-coordinate of piece
					{
						x_right = x + (j + 1) * scl;
					}
					if (y + i * scl < y_top)			// Topmost y-coordinate of piece
					{
						y_top = y + i * scl;
					}
					if (y + (i + 1) * scl > y_bottom)   // Bottom-most coordinates of piece
					{
						y_bottom = y + (i + 1) * scl;
						bottom_row = row + i;
					}
				}
			}
		}
		// Keeping the piece within the boundaries
		if (x_right > right_bound)						// x > right boundary?
		{
			int diff = x_right - right_bound;
			x -= diff;
		}
		if (x_left < left_bound)						// x < left boundary?
		{
			int diff = left_bound - x_left;
			x += diff;
		}

		if (!key[KEY_SPACE])
		{
			anti_spacelock = false;
		}

		if (bottom_row < height - 1)						// Collision with occupied square
		{
			lastMinuteMove = false;

			for (int i = 0; i < piece.size(); i++)
			{
				for (int j = 0; j < piece[0].size(); j++)
				{
					if (piece[i][j] == true)
					{
						if (col + j < width)
						{
							if (ga.gameBoard[row + i + 1][col + j] == true)
							{
								if (key[KEY_LEFT] || key[KEY_RIGHT])  // Last-minute-move mechanism
								{
									lastMinuteMove = true;
									lastMinBuffer--;

									if (lastMinBuffer <= 0)
									{
										ga.setOccupation(piece, color, row, col);
										active = false;
									}
								}
								else
								{
									ga.setOccupation(piece, color, row, col);
									active = false;
								}
							}
						}
					}
				}
			}
		}

		else if (bottom_row <= height)										// If piece collides with ground
		{
			if (key[KEY_LEFT] || key[KEY_RIGHT])								// Last-minute-move mechanism
			{
				lastMinuteMove = true;
				lastMinBuffer--;

				if (lastMinBuffer <= 0)
				{
					ga.setOccupation(piece, color, row, col);
					active = false;
				}
			}
			else
			{
				ga.setOccupation(piece, color, row, col);
				active = false;
			}
		}


		if (!lastMinuteMove && !space_pressed)	// Add falling 
		{
			if (y_bottom < bottom_bound)
			{
				y += dy;
			}
			else
			{
				int diff = y_bottom - bottom_bound;
				y -= diff;
			}
		}

		draw(source, ga);			// Draw piece
	}*/
}

void Shape::controls(GameArea ga)
{
	if (keypressed())
	{
		if (key[KEY_SPACE] && !space_pressed && !anti_spacelock)			// Drop piece 
		{
			updateRowAndColumn();
			int min_dist = getMinDist(ga);
			y = (min_dist + row) * scl + upper_bound;
			space_pressed = true;
		}
		if (!space_pressed)
		{
			if (key[KEY_DOWN] && !lastMinuteMove && bottom_row() < height - 1)			// Move piece in y direction
			{
				if (bottom_row() + 1 < height)
				{
					if (y_bottom() + scl >= bottom_bound)
					{
						int diff = y_bottom() - y;
						y = bottom_bound - diff;
					}
					else
					{
						y += 10 * dy;
					}
				}
			}

			checkIfBlocked(ga);

			if (key[KEY_LEFT] && !left_blocked)			// Move piece in -x direction
			{
				pos_x_buffer = stdbuff;

				if (neg_x_buffer == stdbuff || neg_x_buffer <= 0)
				{
					if (x_left() - scl <= left_bound)
					{
						x = left_bound - (x_left() - x);
					}
					else
					{
						x -= dx;
					}

				}
				neg_x_buffer--;
			}
			else if (key[KEY_RIGHT] && !right_blocked)	// Move piece in +x direction
			{
				neg_x_buffer = stdbuff;

				if (pos_x_buffer == stdbuff || pos_x_buffer <= 0)
				{
					if (x_right() + scl >= right_bound)
					{
						x = right_bound - (x_right() - x);
					}
					else
					{
						x += dx;
					}

				}
				pos_x_buffer--;
			}
			else
			{
				pos_x_buffer = stdbuff;
				neg_x_buffer = stdbuff;
			}
			if (key[KEY_UP])			// Rotate piece clockwise
			{
				if (!repeated)
				{
					rotate(ga);
					rotation++;
					rotation %= 3;
					repeated = true;
				}
			}
			else
			{
				repeated = false;
			}
			if (key[KEY_C] && !anti_erase && skipPiece)
			{
				erase = true;
				skipPiece = false;
			}
		}
	}
}

void Shape::rotate(GameArea ga)
{
	int newRows = piece[0].size();
	int newCols = piece.size();
	vector<vector<bool>> newPiece(newRows, vector<bool>(newCols));

	switch (rotation)
	{
	case 0:
		for (int i = 0; i < newRows; i++)
		{
			for (int j = 0; j < newCols; j++)
			{
				newPiece[i][j] = piece[newCols - 1 -j][i];
			}
		}
		break;

	case 1:
		for (int i = 0; i < newRows; i++)
		{
			for (int j = newCols - 1; j >= 0; j--)
			{
				newPiece[i][j] = piece[newCols - 1 - j][i];
			}
		}
		break;

	case 2:
		for (int i = 0; i < newRows; i++)
		{
			for (int j = 0; j < newCols; j++)
			{
				newPiece[i][j] = piece[newCols - 1 - j][i];
			}
		}
		break;

	default:
		EXIT_FAILURE; // invalid rotation
	}

	if (!illegalCollision(ga, newPiece))
	{
		piece = newPiece;
		rotation--;
	}

	adjustToBoundaries();
}

void Shape::draw(BITMAP *source, GameArea ga)
{
	int x_draw, y_draw;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] == true)
			{
				y_draw = upper_bound + row * scl;
				x_draw = left_bound + col * scl;

				if (!space_pressed)
				{
					square(source, x_draw + j * scl, y_draw + scl * (i + getMinDist(ga)), makecol(160, 206, 230), true); // Ghost piece
				}
				square(source, x_draw + j * scl, y_draw + i * scl, color, true); // Active piece
			}

		}
	}
}

void Shape::checkIfBlocked(GameArea ga)
{
	left_blocked = false;
	right_blocked = false;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (col + j - 1 >= 0)
			{
				if (piece[i][j] && ga.gameBoard[row + i][col + j - 1])
				{
					left_blocked = true;
				}
			}
			if (col + j + 1 < width)
			{
				if (piece[i][j] && ga.gameBoard[row + i][col + j + 1])
				{
					right_blocked = true;
				}
			}
		}
	}
}

void Shape::addFalling(GameArea ga)
{
	if (!groundCollision(ga) && !space_pressed)
	{
		y += dy + level / 3;
	}
}

void Shape::adjustToBoundaries()
{
	if (right_col() >= width)
	{
		int diff = x_right() - right_bound;
		x -= diff;
	}
	else if (left_col() < 0)
	{
		int diff = left_bound - x_left();
		x += diff;
	}
}

void Shape::antiKeyLock()
{
	if (!key[KEY_SPACE])
	{
		anti_spacelock = false;
	}
	if (!key[KEY_C])
	{
		anti_erase = false;
	}
}

void Shape::updateRowAndColumn()
{
	row = floor((y - upper_bound) / scl);
	col = floor((x - left_bound) / scl);
}

void Shape::floatPieceLeft()
{

	int n = 0;
	for (int i = 0; i < piece.size(); i++)
	{
		if (!piece[i][0])
		{
			n++;
		}
	}
	if (n == piece.size())
	{
		for (int i = 0; i < piece.size(); i++)
		{
			for (int j = 0; j < piece[0].size(); j++)
			{
				if (piece[i][j])
				{
					piece[i][j - 1] = piece[i][j];
					piece[i][j] = false;
				}
			}
		}
	}
	
}



bool Shape::groundCollision(GameArea ga)
{
	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j])
			{
				if (ga.gameBoard[row + i + 1][col + j])	// Collision with occupied square
				{
					return true;
				}
				else if (bottom_row() >= height - 1)	// Collision with ground
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool Shape::illegalCollision(GameArea ga, vector<vector<bool>> piece)
{
	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && ga.gameBoard[row + i][col + j]) // Collision with landed piece
			{
				return true;
			}
			if (left_col() < 0 || right_col() >= width || bottom_row() >= height)
			{
				return true;
			}
		}
	}
	return false;
}



int Shape::getMinDist(GameArea ga)
{
	int min_dist = (height - 1) - bottom_row();

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] == true)
			{
				int thisRow = row + i;
				int thisCol = col + j;

				for (int k = thisRow + 1; k < height; k++)
				{
					if (thisCol < width)
					{
						if (ga.gameBoard[k][thisCol] == true)
						{
							if ((k - 1) - thisRow < min_dist)
							{
								min_dist = (k - 1) - thisRow;
								break;
							}
						}

					}
				}
			}
		}
	}

	return min_dist;
}

int Shape::x_left()
{
	return left_bound + col * scl;
}

int Shape::x_right()
{
	int x_right = 0;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && j > x_right)
			{
				x_right = j;
			}
		}
	}
	return left_bound + (col + x_right + 1) * scl;
}

int Shape::y_bottom()
{
	int y_bottom = 0; 

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && i > y_bottom)
			{
				y_bottom = i;
			}
		}
	}
	return upper_bound + (row + y_bottom + 1) * scl;
}

int Shape::y_top()
{
	int y_top = height;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && i < y_top)
			{
				y_top = i;
			}
		}
	}
	return upper_bound + (row + y_top) * scl;
}

int Shape::bottom_row()
{
	int bottom_row = 0;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && i > bottom_row)
			{
				bottom_row = i;
			}
		}
	}
	return row + bottom_row;
}

int Shape::left_col()
{
	return col;
}

int Shape::right_col()
{
	int right_col = 0;

	for (int i = 0; i < piece.size(); i++)
	{
		for (int j = 0; j < piece[0].size(); j++)
		{
			if (piece[i][j] && j > right_col)
			{
				right_col = j;
			}
		}
	}
	return col + right_col;
}



void init();
void deinit();


int main() 
{

	init();
	LOCK_VARIABLE(counter);
	LOCK_FUNCTION(increment);
	install_int_ex(increment, BPS_TO_TIMER(30));
	install_keyboard();


	srand(time(0));
	BITMAP *bmp = create_bitmap(scr_w, scr_h);


	//SAMPLE *tetrisTheme = load_sample("tetristheme.wav");
	//play_sample(tetrisTheme, 20, 127, 1000, 1);


	GameArea gameArea;
	
	int type = rand() % 7;
	Shape activeShape(type, true, gameArea);

	bool running = true;

	
	while (!gameOver && running)
	{
		
		while (counter > 0)
		{

			// Clear bitmap
			clear_bitmap(bmp);


			if (keypressed())
			{
				if (key[KEY_ESC])
				{
					running = false;
				}
			}


			// Draw Background
			rectfill(bmp, 0, 0, scr_w, scr_h, makecol(255, 255, 255));


			// Draw gameArea
			gameArea.draw(bmp);

			// Update game area
			gameArea.update();
			
			// Text output score
			_itoa_s(score, scoreHolder, 10);
			textout_ex(bmp, font, "Score: ", right_bound + 50, upper_bound, makecol(0, 0, 0), -1);
			textout_ex(bmp, font, scoreHolder, right_bound + 100, upper_bound, makecol(0, 0, 0), -1);

			// Text output levl
			_itoa_s(level, levelHolder, 10);
			textout_ex(bmp, font, "Level: ", left_bound - 100, upper_bound, makecol(0, 0, 0), -1);
			textout_ex(bmp, font, levelHolder, left_bound - 50, upper_bound, makecol(0, 0, 0), -1);

			// If shape is inactive, create new shape
			if (!activeShape.isActive() || activeShape.erase)
			{
				type = rand() % 7;
				activeShape = Shape(type, true, gameArea);
				//play_sample(scoreSample, 20, 127, 1000, 0);
			}


			// Handle keyboard input, update and draw shape
			activeShape.controls(gameArea);
			activeShape.update(bmp, gameArea);
			activeShape.draw(bmp, gameArea);
			

			rect(bmp, left_bound, upper_bound, right_bound, bottom_bound, makecol(0,0,0));


			// Blit to screen
			blit(bmp,  screen, 0, 0, 0, 0, scr_w, scr_h);

			counter--;
		}
	}
	while (running)
	{
		
		while (counter > 0)
		{
			// Clear bitmap
			clear_bitmap(bmp);

			rectfill(bmp, 0, 0, scr_w, scr_h, makecol(255, 255, 255));

			if (keypressed())
			{
				if (key[KEY_ESC])
				{
					running = false;
				}
			}

			textout_ex(bmp, font, "Game Over", scr_w / 2, scr_h / 2, makecol(0, 0, 0), -1);


			// Blit to screen
			blit(bmp, screen, 0, 0, 0, 0, scr_w, scr_h);


			counter--;
		}
	}


	deinit();
	return 0;
}


END_OF_MAIN()

void init() {
	int depth, res;
	allegro_init();
	depth = desktop_color_depth();
	if (depth == 0) depth = 32;
	set_color_depth(depth);
	res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, scr_w, scr_h, 0, 0);
	if (res != 0) {
		allegro_message(allegro_error);
		exit(-1);
	}

	install_timer();
	install_keyboard();
	install_mouse();
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
	/* add other initializations here */
}

void deinit() {
	clear_keybuf();
	/* add orther deinitializations here */
}


