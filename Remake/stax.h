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

#ifndef STAX_H
#define STAX_H

#include <allegro.h>
#include <vector>
#include <list>
#include <new>
#include <cstdio>

const int POP_FLASH_TIME = 250;
const int POP_TIME = 3000;

const int BUFFER_WIDTH = 800;
const int BUFFER_HEIGHT = 600;

const int BLOCK_SIZE = 20;
const int NUMBER_OF_BLOCK_BITMAPS = 5;
const int FALL_FRAMES = 4;
const int POP_FRAMES = 6;

const int PANEL_WIDTH = 15;
const int PANEL_HEIGHT = 20;
const int PANEL_START_Y = 135;
const int PANEL1_START_X = 65;
const int PANEL2_START_X = 430;
const int SINGLE_PLAYER_PANEL_START_X = 250;
const int BLOCK_MINIMUM_Y = -BLOCK_SIZE;

const float RISE_START = 0.0005;
const float RISE_INCREMENT = 0.000000001;
const float SHIFTY_RISE_INCREMENT = 0.000000005;

const float FALL_INCREMENT = 0.2;
const float SUCK_INCREMENT = -0.3;
const float BLAST_INCREMENT = 0.5;

const int ROTATE_DELAY = 200;
const int POP_EFFECT_DELAY = 50;
const int BONUS_DELAY = 2000;

const int TIMESTEP = 40;

const int START_CHARGES = 25; // for SpringShot
const int FREE_CHARGE_DELAY = 2000;

const int MAX_STRING = 1000;

struct Block {
	int type;
	bool popping;
	int pop_count;
	int y; // Only used if the block is falling
	float fall_increment; // How fast the block is falling
	float fall_count; // Is the block ready to move down a pixel?
	int pop_power; // how many blocks this one can blast through
};

class Effect {
public:
	virtual void draw(BITMAP *bmp) = 0;
	virtual bool done(int step) = 0;
};

class Panel {
public:
	void shiftTopFallingBlockLeft(int c);
	void shiftTopFallingBlockRight(int c);
	void SwapBlocks(int row, int c1, int c2);
	void SetRiseIncrement(float incr);
	bool ShiftRowLeft(int row);
	bool ShiftRowRight(int row);
	int GetBottomHeight(void);
	Block* GetTopFallingBlock(int column);
	void RemoveTopFallingBlock(int column);
	void FreeTopLandedBlock(int column);
	bool AddFallingBlock(int column, Block* block, bool force = false);
	int GetScore(void);
	bool MoveBlocks(int step); // Returns true when the blocks reach the top of the panel
	void Draw(void);
	Panel(int xx, int yy, int initial_height, float initial_increment);
	~Panel();
private:
	void FreeLandedBlock(int column, int row);
	void FreeColumn(int column, int row);
	int x, y;
	std::vector<Block*> falling_blocks[PANEL_WIDTH];
	Block landed_blocks[PANEL_WIDTH][PANEL_HEIGHT+1];
	int bottom_height;
	float current_increment;
	float raise_count;
	int score;
	float rise_increment;
	std::list<Effect*> effects;
};

const int MIN_REPEAT = 25;
const int MOVE_ACCEL = 25;

class Input {
public:
	virtual bool LeftPressed(void) = 0;
	virtual bool RightPressed(void) = 0;
	virtual bool UpPressed(void) = 0;
	virtual bool DownPressed(void) = 0;
	virtual bool ButtonPressed(void) = 0;
	bool LeftReady(void);
	bool RightReady(void);
	bool UpReady(void);
	bool DownReady(void);
	bool ButtonReady(void);
	void PressLeft(void);
	void PressRight(void);
	void PressUp(void);
	void PressDown(void);
	void PressButton(void);
	void SetRepeatTimes(int left, int right, int up, int down, int button);
	virtual void Update(void);
	Input(void);
protected:
	int left_repeat;
	int left_count;
	int right_repeat;
	int right_count;
	int up_repeat;
	int up_count;
	int down_repeat;
	int down_count;
	int button_repeat;
	int button_count;
	int init_left_repeat;
	int init_right_repeat;
	int init_up_repeat;
	int init_down_repeat;
	int init_button_repeat;
};

class LeftKeyboardInput : public Input {
	bool LeftPressed(void);
	bool RightPressed(void);
	bool UpPressed(void);
	bool DownPressed(void);
	bool ButtonPressed(void);
	void Update(void);
};

class RightKeyboardInput : public Input {
	bool LeftPressed(void);
	bool RightPressed(void);
	bool UpPressed(void);
	bool DownPressed(void);
	bool ButtonPressed(void);
	void Update(void);
};

class Joystick1Input : public Input {
	bool LeftPressed(void);
	bool RightPressed(void);
	bool UpPressed(void);
	bool DownPressed(void);
	bool ButtonPressed(void);
	void Update(void);
};

class Joystick2Input : public Input {
	bool LeftPressed(void);
	bool RightPressed(void);
	bool UpPressed(void);
	bool DownPressed(void);
	bool ButtonPressed(void);
	void Update(void);
};

const int NUMBER_OF_GAME_TYPES = 3;
enum GameType { GAME_TYPE_SUCKER = 0, GAME_TYPE_SPRING_SHOT, GAME_TYPE_SHIFTY };

const int NUMBER_OF_INPUT_TYPES = 5;
enum InputType { INPUT_NONE = 0, INPUT_RIGHT_KEYBOARD, INPUT_LEFT_KEYBOARD, INPUT_JOYSTICK_1, INPUT_JOYSTICK_2 };

const int NUM_CONFIGURABLE_COLORS = 11;
enum CONFIGURABLE_COLORS {
	COLOR_TOP = 0,
	COLOR_BOTTOM = 1,
	COLOR_BORDER = 2,
	COLOR_BORDER_MIDDLE = 3,
	COLOR_SELECTED_BORDER = 4,
	COLOR_SELECTED_BORDER_MIDDLE = 5,
	COLOR_UNSELECTED = 6,
	COLOR_TEXT = 7,
	COLOR_TEXT_SELECTED = 8,
	COLOR_TEXT_DEPRESSED = 9,
	COLOR_TEXT_OUTLINE = 10
};

struct Configuration {
	int graphics_driver;
	bool graphics_mode_set;
	int screen_width;
	int screen_height;
	int color_depth;
	GameType game_type;
	int number_of_block_types;
	int initial_height;
	InputType input_type_1;
	InputType input_type_2;
	int joystick1_button;
	int joystick2_button;
	int sound_driver;
	int midi_driver;
	bool sound_enabled;
	int sound_volume;
	int music_volume;
	char* color_scheme;
	RGB colors[NUM_CONFIGURABLE_COLORS];
	bool mouse_installed;
	bool joystick_installed;
};

enum WidgetType {
	WIDGET_END = 0,
	WIDGET_EDITOR = 1,
	WIDGET_SLIDER = 2,
	WIDGET_TEXT = 3,
	WIDGET_CHECKBOX = 4,
	WIDGET_BUTTON = 5,
	WIDGET_TEXTBUTTON = 6,
	WIDGET_ICON = 7,
	WIDGET_CHAR_SELECTOR = 8,
	WIDGET_OPTION = 9,
	WIDGET_LIST = 10
};

enum WidgetState {
	WIDGET_STATE_UNSELECTED = 0,
	WIDGET_STATE_SELECTED = 1,
	WIDGET_STATE_DEPRESSED = 2,
	WIDGET_STATE_LEFT = 3,
	WIDGET_STATE_RIGHT = 4,
	WIDGET_STATE_LEFT_DEPRESSED = 5,
	WIDGET_STATE_RIGHT_DEPRESSED = 6
};

enum AlignType {
	ALIGN_LEFT = 0,
	ALIGN_CENTER = 1,
	ALIGN_RIGHT = 2
};

enum ValidCharType {
	VALID_NUM = 0,
	VALID_UPPERCASE = 1,
	VALID_NAME = 2
};

typedef struct {
	WidgetType type;
	AlignType align;
	int x, y;
	int w, h;
	char *s;
	int d1, d2, d3, d4;
	void *p1, *p2, *p3;
	int state;
} Widget;

typedef char *(*OptionGetter)(int, int *);

typedef struct {
	int x, y;
} Point;

const int NUM_HIGH_SCORES = 10;
const int MAX_NAME = 20;

struct HighScore {
	char name[MAX_NAME+1];
	int score;
	int blocks; // Number of blocks the user played with
};

// Exceptions
class BadGraphicsMode {};
class BadInstall {};
class BadLoad {};
class Invalid {};

extern BITMAP* buffer;
extern Configuration configuration;
//extern volatile int tick;
extern Panel* panel1;
extern Panel* panel2;
extern Input* input1;
extern Input* input2;

extern DATAFILE* datafile;
extern BITMAP* block_bitmaps[NUMBER_OF_BLOCK_BITMAPS];
extern BITMAP* falling_block_bitmaps[NUMBER_OF_BLOCK_BITMAPS][FALL_FRAMES];
extern BITMAP* popping_block_bitmaps[NUMBER_OF_BLOCK_BITMAPS];
extern BITMAP* pop_bitmaps[NUMBER_OF_BLOCK_BITMAPS][POP_FRAMES];
extern BITMAP* magnet_bitmap;
extern BITMAP* swapper_bitmap;
extern BITMAP* spring_bitmaps[4];
extern BITMAP* arrow_r_bitmap;
extern BITMAP* arrow_l_bitmap;
extern SAMPLE* pop_sample;
extern SAMPLE* suck_sample;
extern SAMPLE* thunk_sample;
extern SAMPLE* toggle_sample;
extern SAMPLE* charge_sample;
extern SAMPLE* blast_sample;
extern SAMPLE* nocharge_sample;
extern MIDI* intro_midi;
extern MIDI* game_midi;
extern BITMAP* arrow_left, *arrow_left_selected, *arrow_left_depressed;
extern BITMAP* arrow_right, *arrow_right_selected, *arrow_right_depressed;
extern BITMAP* check_on, *check_on_selected, *check_on_depressed;
extern BITMAP* check_off, *check_off_selected, *check_off_depressed;
extern BITMAP* radio_on, *radio_on_selected, *radio_on_depressed;
extern BITMAP* radio_off, *radio_off_selected, *radio_off_depressed;
extern BITMAP* slider_tab, *slider_tab_selected, *slider_tab_depressed;
extern bool in_gui;
extern BITMAP* background_bitmap;
extern HighScore high_scores[NUMBER_OF_GAME_TYPES][NUM_HIGH_SCORES];
extern char* high_score_filename;
extern int falling_block_frame;

// Game loops
extern bool Sucker(void);
extern bool SpringShot(void);
extern bool Shifty(void);

extern void ProcessCommandLine(int argc, char** argv);

extern void AddColorScheme(char* name, RGB colors[NUM_CONFIGURABLE_COLORS]);
extern void SetColorScheme(char* name);
extern int GetNumColorSchemes(void);
extern char* GetColorSchemeName(unsigned int i);
extern void Configure(void);
extern void ReadConfiguration(char* argv0);
extern void WriteConfiguration(char* argv0);

extern void LoadData(void);
extern void DestroyData(void);

extern void SetGraphicsMode(void);
extern void CreateBuffer(void);
extern void DestroyBuffer(void);
extern void DrawBlock(int x, int y, Block* block, bool falling);
extern void FillRectangle(int x1, int y1, int x2, int y2);
extern void DrawSunkenRectangle(int x1, int y1, int x2, int y2);
extern void BlitToScreen(void);
extern bool IsSupportedColorDepth(int color_depth);
extern void DrawText(int x, int y, bool center, char* format, ...);
extern void ChangeColor(BITMAP *b, int from, int to);
extern void DrawBackgroundBitmap(void);
extern void SaveScreenshot(void);

void textout_outline(BITMAP *b, FONT *f, char *s, int x, int y, int color, int outline_color, int bg);
extern int GUI_GetGradientColor(float percent, int top_color, int bottom_color);
extern void GUI_Message(char* s1, char* s2, char* s3);
extern void GUI_WaitMessage(char *s, ...);
extern bool GUI_Prompt(char *message, char *b1, char *b2);
extern int GUI_Go(int x, int y, int w, int h, Widget *widgets, int selected, void (*callback)(bool));
extern void GUI_GetName(char *caption, char *buf, int maxlen);
extern void GUI_Initialize(void);
extern void GUI_ShutDown(void);
extern int GUI_TOP_COLOR;
extern int GUI_BOTTOM_COLOR;
extern int GUI_BORDER_COLOR;
extern int GUI_BORDER_COLOR_MIDDLE;
extern int GUI_SELECTED_BORDER_COLOR;
extern int GUI_SELECTED_BORDER_COLOR_MIDDLE;
extern int GUI_UNSELECTED_COLOR;
extern int GUI_TEXT_COLOR;
extern int GUI_SELECTED_TEXT_COLOR;
extern int GUI_DEPRESSED_TEXT_COLOR;
extern int GUI_BASE_COLOR;
extern int GUI_TEXT_OUTLINE_COLOR;

extern char* GetHighScoresFilename(char* argv0);
extern int ReadHighScores(char* filename);
extern int WriteHighScores(char* filename);
extern void ViewHighScores(void);
extern void CheckHighScores(int pops);

extern bool LeftPressed(void);
extern bool RightPressed(void);
extern bool UpPressed(void);
extern bool DownPressed(void);
extern bool ButtonPressed(void);
extern void WaitForRelease(void);
extern Input* CreateInput(int player);

extern int GetRandomBlock(void);

extern void InstallSound(void);
extern void RemoveSound(void);
extern void StopMusic(void);
extern void PlayIntroMusic(void);
extern void PlayGameMusic(void);

/*
extern void InstallTimer(void);
extern void RemoveTimer(void);
*/
extern void UpdateFallingBlocks(int step);
extern long currentTimeMillis(void);

#endif // STAX_H
