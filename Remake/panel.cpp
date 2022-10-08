/*
 * Copyright 1999-2006 Trent Gamblin
 *
 * This file is part of Stax.
 *
 * Stax is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cstdlib>

#include "stax.h"

class PopEffect : public Effect
{
public:
	void draw(BITMAP* bmp)
	{
		draw_sprite(bmp, pop_bitmaps[type][currFrame], x, y);
	}
	bool done(int step)
	{
		changeCount += step;
		if (changeCount > POP_EFFECT_DELAY) {
			changeCount -= POP_EFFECT_DELAY;
			currFrame++;
			if (currFrame >= POP_FRAMES)
				return true;
		}
		return false;
	}
	PopEffect(int x_, int y_, int type_) {
		x = x_;
		y = y_;
		type = type_;
		changeCount = 0;
		currFrame = 0;
	}
private:
	int x;
	int y;
	int changeCount;
	int currFrame;
	int type;
};

class BonusEffect : public Effect
{
public:
	void draw(BITMAP* b)
	{
		textout_ex(b, font, text, x-1, y-1, makecol(0, 0, 0), -1);
		textout_ex(b, font, text, x+1, y-1, makecol(0, 0, 0), -1);
		textout_ex(b, font, text, x-1, y+1, makecol(0, 0, 0), -1);
		textout_ex(b, font, text, x+1, y+1, makecol(0, 0, 0), -1);
		textout_ex(b, font, text, x, y, color, -1);
	}

	bool done(int step)
	{
		count += step;
		if (count > BONUS_DELAY)
			return true;
		return false;
	}

	BonusEffect(int x_, int y_, char* text_, int col)
	{
		text = text_;
		x = x_;
		y = y_;
		color = col;
		count = 0;
	}

	~BonusEffect()
	{
		delete[] text;
	}
private:
	int x;
	int y;
	char* text;
	int color;
	int count;
};

int GetRandomBlock(void)
{
	return rand() % configuration.number_of_block_types;
}

static void GenerateRow(int row, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	for (int x = 0; x < PANEL_WIDTH; x++) {
		int block = rand() % configuration.number_of_block_types;
		blocks[x][row].type = block;
		blocks[x][row].popping = false;
		blocks[x][row].pop_count = 0;
		blocks[x][row].pop_power = 0;
	}
}

static bool Check3Horizontal(int x1, int x2, int x3, int y, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	if (x1 < 0 || x3 >= PANEL_WIDTH || y < 0 || y > PANEL_HEIGHT)
		return false;

	bool popping = (blocks[x1][y].type == blocks[x2][y].type) && (blocks[x2][y].type == blocks[x3][y].type);

	if (popping) {
		int min_count;
		min_count = MIN(blocks[x1][y].pop_count, blocks[x2][y].pop_count);
		min_count = MIN(min_count, blocks[x3][y].pop_count);
		blocks[x1][y].pop_count = min_count;
		blocks[x2][y].pop_count = min_count;
		blocks[x3][y].pop_count = min_count;
	}

	return popping;
}

static bool Check3Vertical(int x, int y1, int y2, int y3, int end_row, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	if (y1 < 0 || y3 > end_row || x < 0 || x >= PANEL_WIDTH)
		return false;
	
	bool popping = (blocks[x][y1].type == blocks[x][y2].type) && (blocks[x][y2].type == blocks[x][y3].type);

	if (popping) {
		int min_count;
		min_count = MIN(blocks[x][y1].pop_count, blocks[x][y2].pop_count);
		min_count = MIN(min_count, blocks[x][y3].pop_count);
		blocks[x][y1].pop_count = min_count;
		blocks[x][y2].pop_count = min_count;
		blocks[x][y3].pop_count = min_count;
	}

	return popping;
}

static bool BlockShouldBePopping(int x, int y, int end_row, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	return Check3Horizontal(x-2, x-1, x, y, blocks) |
		Check3Horizontal(x-1, x, x+1, y, blocks) |
		Check3Horizontal(x, x+1, x+2, y, blocks) |
		Check3Vertical(x, y-2, y-1, y, end_row, blocks) |
		Check3Vertical(x, y-1, y, y+1, end_row, blocks) |
		Check3Vertical(x, y, y+1, y+2, end_row, blocks);
}

static int MarkPopping(int start_row, int end_row, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	int count = 0;

	for (int y = start_row; y <= end_row; y++)
		for (int x = 0; x < PANEL_WIDTH; x++) {
			if (blocks[x][y].type < 0)
				continue;
			if (BlockShouldBePopping(x, y, end_row, blocks)) {
				blocks[x][y].popping = true;
				count++;
			}
			else {
				blocks[x][y].popping = false;
				blocks[x][y].pop_count = 0;
			}
		}

	return count;
}

static void RegeneratePopping(int start_row, int end_row, Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1])
{
	for (int y = start_row; y <= end_row; y++)
		for (int x = 0; x < PANEL_WIDTH; x++)
			if (blocks[x][y].popping) {
				blocks[x][y].type = rand() % configuration.number_of_block_types;
				blocks[x][y].popping = false;
			}
}

static bool GameIsOver(Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1], int bottom_height)
{
	for (int x = 0; x < PANEL_WIDTH; x++)
		if (blocks[x][0].type >= 0)
			return true;

	return false;
}

static void LandBlock(Block blocks[PANEL_WIDTH][PANEL_HEIGHT+1], int column, int row, Block* block)
{
	blocks[column][row].type = block->type;
	blocks[column][row].popping = false;
	blocks[column][row].pop_count = 0;
}

void Panel::shiftTopFallingBlockLeft(int c)
{
	if (falling_blocks[c-1].size())
		return;

	Block* b = GetTopFallingBlock(c);

	if (!b)
		return;

	int by = b->y - bottom_height;

	if (landed_blocks[c-1][by/BLOCK_SIZE].type >= 0 ||
			landed_blocks[c-1][by/BLOCK_SIZE+1].type >= 0)
		return;

	Block* n = new Block;
	
	memcpy(n, b, sizeof(Block));
	RemoveTopFallingBlock(c);

	falling_blocks[c-1].push_back(n);
}

void Panel::shiftTopFallingBlockRight(int c)
{
	if (falling_blocks[c+1].size())
		return;
	
	Block* b = GetTopFallingBlock(c);

	if (!b)
		return;

	int by = b->y - bottom_height;

	if (landed_blocks[c+1][by/BLOCK_SIZE].type >= 0 ||
			landed_blocks[c+1][by/BLOCK_SIZE+1].type >= 0)
		return;

	Block* n = new Block;
	
	memcpy(n, b, sizeof(Block));
	RemoveTopFallingBlock(c);

	falling_blocks[c+1].push_back(n);
}

void Panel::SwapBlocks(int row, int c1, int c2)
{
	int tmp = landed_blocks[c1][row].type;
	landed_blocks[c1][row].type = landed_blocks[c2][row].type;
	landed_blocks[c2][row].type = tmp;
}

void Panel::FreeLandedBlock(int column, int row)
{
	Block* block = new Block;
	block->type = landed_blocks[column][row].type;
	block->popping = false;
	block->pop_count = 0;
	block->y = (row*BLOCK_SIZE) - bottom_height - 1;
	block->fall_increment = FALL_INCREMENT;
	block->fall_count = 0.0;
	block->pop_power = 0;
	AddFallingBlock(column, block, true);
	landed_blocks[column][row].type = -1;
}

/*
 * Free every block in a column starting at
 * row "row" and moving up.
 */
void Panel::FreeColumn(int column, int row)
{
	for (int i = row; i >= 0; i--) {
		if (landed_blocks[column][i].type >= 0)
			FreeLandedBlock(column, i);
	}
}

void Panel::SetRiseIncrement(float incr)
{
	rise_increment = incr;
}

bool Panel::ShiftRowLeft(int row)
{
	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (falling_blocks[i].size() > 0)
			return false;
	}

	int firstblock = landed_blocks[0][row].type;

	for (int i = 0; i < PANEL_WIDTH; i++) {
		int block;
		if (i == PANEL_WIDTH-1)
			block = firstblock;
		else
			block = landed_blocks[i+1][row].type;
		landed_blocks[i][row].type = block;
		landed_blocks[i][row].popping = false;
	}

	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (landed_blocks[i][row].type == -1) {
			FreeColumn(i, row-1);
		}
	}
	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (landed_blocks[i][row+1].type == -1) {
			FreeColumn(i, row);
		}
	}

	return true;
}

bool Panel::ShiftRowRight(int row)
{
	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (falling_blocks[i].size() > 0)
			return false;
	}

	int lastblock = landed_blocks[PANEL_WIDTH-1][row].type;

	for (int i = PANEL_WIDTH-1; i >= 0; i--) {
		int block;
		if (i == 0)
			block = lastblock;
		else
			block = landed_blocks[i-1][row].type;
		landed_blocks[i][row].type = block;
		landed_blocks[i][row].popping = false;
	}

	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (landed_blocks[i][row].type == -1) {
			FreeColumn(i, row-1);
		}
	}
	for (int i = 0; i < PANEL_WIDTH; i++) {
		if (landed_blocks[i][row+1].type == -1) {
			FreeColumn(i, row);
		}
	}

	return true;
}

int Panel::GetBottomHeight(void)
{
	return bottom_height;
}

Block* Panel::GetTopFallingBlock(int column)
{
	if (falling_blocks[column].size() == 0)
		return 0;

	int highest = 0;

	for (unsigned int i = 0; i < falling_blocks[column].size(); i++) {
		if (falling_blocks[column][i]->y < falling_blocks[column][highest]->y)
			highest = i;
	}

	return falling_blocks[column][highest];
}

void Panel::RemoveTopFallingBlock(int column)
{
	Block* block = GetTopFallingBlock(column);
	if (block == 0)
		return;

	unsigned int i;

	for (i = 0; i < falling_blocks[column].size(); i++)
		if (block == falling_blocks[column][i])
			break;

	delete block;
	std::vector<Block*>::iterator it = falling_blocks[column].begin() + i;
	falling_blocks[column].erase(it);
}

// FIXME: call FreeLandedBlock(..)
void Panel::FreeTopLandedBlock(int column)
{
	int highest = PANEL_HEIGHT-1;

	while (landed_blocks[column][highest].type != -1)
		highest--;
	highest++;

	if (highest >= PANEL_HEIGHT)
		return;

	Block* block = new Block;
	block->type = landed_blocks[column][highest].type;
	block->popping = false;
	block->pop_count = 0;
	block->y = (highest*BLOCK_SIZE) - bottom_height - 1;
	block->fall_increment = 0.0;
	block->fall_count = 0.0;
	block->pop_power = 0;
	AddFallingBlock(column, block, true);
	landed_blocks[column][highest].type = -1;
}

// Returns true if the block was added and false if there was another block in the way
bool Panel::AddFallingBlock(int column, Block* block, bool force)
{
	if (!force) {
		// Check that the block won't be on top of another
		for (unsigned int i = 0; i < falling_blocks[column].size(); i++) {
			if ((falling_blocks[column][i]->y >= (block->y+BLOCK_SIZE)) || (falling_blocks[column][i]->y <= (block->y-BLOCK_SIZE)))
				continue;
			return false;
		}
		if ((landed_blocks[column][0].type >= 0) || (bottom_height && (landed_blocks[column][1].type >= 0)))
			return false;
	}

	// Add it to the list
	falling_blocks[column].push_back(block);

	return true;
}

int Panel::GetScore(void)
{
	return score;
}

bool Panel::MoveBlocks(int step) throw (std::bad_alloc)
{
	current_increment += rise_increment * step;

	// Update effects
	std::vector<Effect*> remove;

	std::list<Effect*>::iterator it;
	for (it = effects.begin(); it != effects.end(); it++) {
		Effect* e = *it;
		if (e->done(step))
			remove.push_back(e);
	}

	for (int i = 0; i < (int)remove.size(); i++) {
		effects.remove(remove[i]);
	}

	remove.clear();

	// Move the falling blocks
	for (int c = 0; c < PANEL_WIDTH; c++)
		for (unsigned int i = 0; i < falling_blocks[c].size(); i++) {
			Block* block = falling_blocks[c][i];
			block->fall_count += block->fall_increment * step;
			if (block->fall_count >= 1.0) {
				block->fall_count -= 1.0;
				block->y++;
				// See if there is a block directly below this one
				if (block->y >= (-BLOCK_SIZE) && landed_blocks[c][((block->y+bottom_height)/BLOCK_SIZE)+1].type >= 0) {
					// If this block has any "pop power" destroy the one below it
					if ((((block->y+bottom_height)/BLOCK_SIZE)+1) < PANEL_HEIGHT && block->pop_power) {
						landed_blocks[c][((block->y+bottom_height)/BLOCK_SIZE)+1].type = -1;
						block->pop_power--;
					}
					else {
						LandBlock(landed_blocks, c, ((block->y+bottom_height)/BLOCK_SIZE), block);
						delete block;
						block = 0;
						std::vector<Block*>::iterator it = falling_blocks[c].begin() + i;
						falling_blocks[c].erase(it);
					}
				}
			}
			else if (block->fall_count <= -1.0) {
				block->fall_count += 1.0;
				block->y--;
				if (block->y < BLOCK_MINIMUM_Y)
					block->y = BLOCK_MINIMUM_Y;
			}
			if (block && block->fall_increment < 0)
				block->fall_increment = FALL_INCREMENT;
		}

	// Move the landed blocks
	raise_count += current_increment * step;
	if (raise_count >= 1.0) {
		bottom_height++;
		raise_count -= 1.0;
	}

	if (GameIsOver(landed_blocks, bottom_height))
		return true;

	if (bottom_height == BLOCK_SIZE) {
		for (int r = 0; r < PANEL_HEIGHT; r++)
			for (int c = 0; c < PANEL_WIDTH; c++)
				landed_blocks[c][r] = landed_blocks[c][r+1];
		GenerateRow(PANEL_HEIGHT, landed_blocks);
		while (MarkPopping(PANEL_HEIGHT, PANEL_HEIGHT, landed_blocks))
			RegeneratePopping(PANEL_HEIGHT, PANEL_HEIGHT, landed_blocks);
		bottom_height = 0;
	}
	MarkPopping(0, PANEL_HEIGHT-1, landed_blocks);

	// Remove blocks that have been flashing long enough

	bool popped = false;
	int pops = 0;

	int popx = 0, popy = 0;
	int popcolor;

	for (int r = 0; r < PANEL_HEIGHT; r++)
		for (int c = 0; c < PANEL_WIDTH; c++)
			if ((landed_blocks[c][r].type >= 0) && landed_blocks[c][r].popping) {
				landed_blocks[c][r].pop_count++;
				if (landed_blocks[c][r].pop_count >= POP_TIME) {
					popped = true;
					int xx = c*BLOCK_SIZE+x;
					int yy = r*BLOCK_SIZE+y-bottom_height;
					int o = (pop_bitmaps[0][0]->w - BLOCK_SIZE) / 2;
					xx -= o;
					yy -= o;
					PopEffect *e = new PopEffect(xx, yy, landed_blocks[c][r].type);
					effects.push_back(e);
					switch (landed_blocks[c][r].type) {
					case 0:
						popcolor = makecol(0, 255, 0);
						break;
					case 1:
						popcolor = makecol(0, 255, 255);
						break;
					case 2:
						popcolor = makecol(200, 0, 200);
						break;
					case 3:
						popcolor = makecol(255, 0, 0);
						break;
					case 4:
						popcolor = makecol(255, 255, 0);
						break;
					}
					landed_blocks[c][r].type = -1;
					pops++;
					popx += c*BLOCK_SIZE;
					popy += r*BLOCK_SIZE;
				}
			}

	if (pops > 0) {
		score += pops + (pops - 3);
		if (pops > 3) {
			popx /= pops;
			popy /= pops;
			char tmp[100];
			sprintf(tmp, "BONUS x %d", pops-3);
			char *text = new char[strlen(tmp)+1];
			strcpy(text, tmp);
			popx -= text_length(font, text) / 2;
			popy -= text_height(font) / 2 + bottom_height;
			popx += x + BLOCK_SIZE/2;
			popy += y + BLOCK_SIZE/2;
			BonusEffect* e = new BonusEffect(popx, popy, text, popcolor);
			effects.push_back(e);
		}
	}

	if (popped)
		play_sample(pop_sample, 255, 0, 1000, 0);

	// Add the hanging blocks to the falling blocks list
	for (int c = 0; c < PANEL_WIDTH; c++) {
		int r;
		for (r = PANEL_HEIGHT-1; (landed_blocks[c][r].type >= 0) && (r >= 0); r--)
			;
		if (r <= 0)
			continue;
		for (; (landed_blocks[c][r].type < 0) && (r >= 0); r--)
			;
		if (r <= 0)
			continue;
		// Everything from r up must now become a falling block
		while (landed_blocks[c][r].type >= 0) {
			Block* block = new Block;
			block->type = landed_blocks[c][r].type;
			block->popping = false;
			block->pop_count = 0;
			block->y = (r*BLOCK_SIZE) - bottom_height;
			block->fall_increment = FALL_INCREMENT;
			block->fall_count = 0.0;
			block->pop_power = 0;
			AddFallingBlock(c, block, true);
			landed_blocks[c][r].type = -1;
			r--;
		}
	}

	return false;
}

void Panel::Draw(void)
{
	FillRectangle(x, y, x+(PANEL_WIDTH*BLOCK_SIZE)-1, y+(PANEL_HEIGHT*BLOCK_SIZE)-1);

	for (int c = 0; c < PANEL_WIDTH; c++)
		for (unsigned int i = 0; i < falling_blocks[c].size(); i++)
			DrawBlock(x+(c*BLOCK_SIZE), y+falling_blocks[c][i]->y, falling_blocks[c][i], true);
	
	for (int r = 0; r < PANEL_HEIGHT+1; r++)
		for (int c = 0; c < PANEL_WIDTH; c++)
			DrawBlock(x+(c*BLOCK_SIZE), y+(r*BLOCK_SIZE)-bottom_height, &landed_blocks[c][r], false);

	std::list<Effect*>::iterator it;

	for (it = effects.begin(); it != effects.end(); it++) {
		Effect* e = *it;
		e->draw(buffer);
	}
}

Panel::~Panel()
{
	for (int i = 0; i < PANEL_WIDTH; i++) {
		for (int j = 0; j < (int)falling_blocks[i].size(); j++) {
			delete falling_blocks[i][j];
		}
		falling_blocks[i].clear();
	}

	std::list<Effect*>::iterator it;
	for (it = effects.begin(); it != effects.end(); it++) {
		Effect* e = *it;
		delete e;
	}
	effects.clear();
}

Panel::Panel(int xx, int yy, int initial_height, float initial_increment)
{
	x = xx;
	y = yy;
	bottom_height = 0;
	current_increment = initial_increment;
	raise_count = 0.0;
	score = 0;
	rise_increment = RISE_INCREMENT;

	for (int r = 0; r < PANEL_HEIGHT+1; r++)
		for (int c = 0; c < PANEL_WIDTH; c++) {
			landed_blocks[c][r].type = -1;
			landed_blocks[c][r].popping = false;
		}

	if (initial_height >= PANEL_HEIGHT)
		initial_height = PANEL_HEIGHT-1;

	for (int i = 0; i < initial_height+1; i++)
		GenerateRow(PANEL_HEIGHT-i, landed_blocks);

	while (MarkPopping(0, PANEL_HEIGHT, landed_blocks))
		RegeneratePopping(0, PANEL_HEIGHT, landed_blocks);
}
