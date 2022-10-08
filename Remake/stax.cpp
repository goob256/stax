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

#include <iostream>

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <allegro.h>

#include "stax.h"

BITMAP* buffer;
static BITMAP* logo;

Configuration configuration = {
	GFX_AUTODETECT_WINDOWED, // graphics_driver
	false, // graphics_mode_set
	800, // screen_width
	600, // screen_height
	32, // color_depth, set to desktop_color_depth() in main
	GAME_TYPE_SUCKER, // game_type
	4, // number_of_block_types
	5, // initial_height
	INPUT_RIGHT_KEYBOARD, // input_type_1
	INPUT_NONE, // input_type_2
	1, // joystick1_button
	1, // joystick2_button
	DIGI_AUTODETECT, // sound_driver
	MIDI_AUTODETECT, // midi_driver
	true, // sound_enabled
	128, // sound_volume
	64, // music_volume
	"Default", // color_scheme
	{ 0, }, // colors
	false, // mouse_installed
	false // joystick installed
};

volatile int tick = 0;

Panel* panel1;
Panel* panel2;

Input* input1;
Input* input2 = 0;

DATAFILE* datafile = 0;
BITMAP* block_bitmaps[NUMBER_OF_BLOCK_BITMAPS];
BITMAP* falling_block_bitmaps[NUMBER_OF_BLOCK_BITMAPS][FALL_FRAMES];
BITMAP* popping_block_bitmaps[NUMBER_OF_BLOCK_BITMAPS];
BITMAP* pop_bitmaps[NUMBER_OF_BLOCK_BITMAPS][POP_FRAMES];
BITMAP* magnet_bitmap;
BITMAP* swapper_bitmap;
BITMAP* spring_bitmaps[4];
BITMAP* arrow_r_bitmap;
BITMAP* arrow_l_bitmap;
SAMPLE* pop_sample;
SAMPLE* suck_sample;
SAMPLE* thunk_sample;
SAMPLE* toggle_sample;
SAMPLE* charge_sample;
SAMPLE* blast_sample;
SAMPLE* nocharge_sample;
MIDI* game_midi;
MIDI* intro_midi;

BITMAP* background_bitmap;

HighScore high_scores[NUMBER_OF_GAME_TYPES][NUM_HIGH_SCORES] = {
	{
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 }
	},
	{
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 }
	},
	{
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 },
		{ "Noone", 0 }
	}
};

char* high_score_filename = 0;

static const int NUM_DEFAULT_COLOR_SCHEMES = 1;

static RGB default_color_schemes[NUM_DEFAULT_COLOR_SCHEMES][NUM_CONFIGURABLE_COLORS] = {
	{
		{ 200, 200, 200 },
		{ 0, 50, 100 },
		{ 100, 100, 100 },
		{ 150, 150, 150 },
		{ 180, 180, 0 },
		{ 255, 255, 0 },
		{ 0, 50, 100 },
		{ 220, 220, 220 },
		{ 0, 255, 0 },
		{ 255, 255, 0 },
		{ 0, 0, 0 }
	}
};

int falling_block_frame;

static char* default_color_scheme_names[NUM_DEFAULT_COLOR_SCHEMES] = {
	"Default"
};

static bool (*game_functions[NUMBER_OF_GAME_TYPES])(void) = {
	Sucker,
	SpringShot,
	Shifty
};

const int logo_w = 15;
const int logo_h = 5;

static void DrawLogo(void)
{
	char *data[] = {
		"111022203330404",
		"100002003030404",
		"111002003330040",
		"001002003030404",
		"111002003030404",
	};

	logo = create_bitmap(logo_w*BLOCK_SIZE, logo_h*BLOCK_SIZE);
	if (!logo)
		return;

	clear(logo);

	for (int y = 0; y < logo_h; y++)
		for (int x = 0; x < logo_w; x++) {
			if (data[y][x]-'0') {
				draw_sprite(logo, block_bitmaps[(data[y][x]-'0')-1], x*BLOCK_SIZE, y*BLOCK_SIZE);
			}
		}
}

static void ShowLogo(bool draw)
{
	static int t = 0;
	static bool first_call = true;
	static float ys[15*20];
	static int start_time = -1;
	static bool wave = true;

	if (start_time < 0) {
		start_time = currentTimeMillis();
	}

	if (first_call) {
		first_call = false;
		float inc = 6.28/(logo_w*BLOCK_SIZE);
		float pos = 0.0;
		for (int i = 0; i < (logo_w*BLOCK_SIZE); i++) {
			ys[i] = sin(pos) * 25.0;
			pos += inc;
		}
	}

	t++;

	if (wave && (t >= 10)) {
		t = 0;
		float firsty = ys[0];
		for (int i = 0; i < (logo_w*BLOCK_SIZE); i++) {
			if (i == (logo_w*BLOCK_SIZE)-1)
				ys[i] = firsty;
			else
				ys[i] = ys[i+1];
		}
	}
	else if (!wave) {
		for (int i = 0; i < (logo_w*BLOCK_SIZE); i++)
			ys[i] = 0;
	}

	if (!draw)
		return;

	clear(buffer);

	const int x = (BUFFER_WIDTH/2) - (logo_w*BLOCK_SIZE/2);
	const int y = 150;

	for (int i = 0; i < (logo_w*BLOCK_SIZE); i++) {
		blit(logo, buffer, i, 0, x+i, y+(int)ys[i], 1, logo_h*BLOCK_SIZE);
	}

	textout_outline(buffer, font, "Program, GFX & Sounds: Trent Gamblin", 10, BUFFER_HEIGHT-text_height(font)-10, makecol(0, 255, 0), makecol(0, 100, 0), -1);
	textout_outline(buffer, font, "Music: Andrew Skrypnyk", BUFFER_WIDTH-text_length(font, "Music: Andrew Skrypnyk")-10, BUFFER_HEIGHT-text_height(font)-10, makecol(255, 0, 0), makecol(100, 0, 0), -1);
}

enum StartMenuChoice { START_MENU_START_GAME, START_MENU_VIEW_HIGH_SCORES, START_MENU_CONFIGURE, START_MENU_EXIT };

int StartMenu(void)
{
	clear(buffer);

	Widget start_menu_widgets[] = {
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 70, 20, 0, 0, "Start Game", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 70, 40, 0, 0, "High Scores", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 70, 60, 0, 0, "Configure", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 70, 80, 0, 0, "Exit", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	int ret = GUI_Go(400-70, 400-60, 140, 120, start_menu_widgets, 0, ShowLogo);

	switch (ret) {
	case 0: return START_MENU_START_GAME;
	case 1: return START_MENU_VIEW_HIGH_SCORES;
	case 2: return START_MENU_CONFIGURE;
	default: return START_MENU_EXIT;
	}
}

int main(int argc, char** argv)
{
	allegro_init();

	install_timer();

	rest(1000); // double click fix for gnome

	configuration.color_depth = desktop_color_depth();

	ProcessCommandLine(argc, argv);

	// Seed the random number generator
	srand(time(NULL));

	/*
	try {
		InstallTimer();
	}
	catch (BadInstall) {
		allegro_message("Error installing timer");
		allegro_exit();
		exit(1);
	}
	*/

	install_keyboard();
	if (install_mouse() >= 0)
		configuration.mouse_installed = true;
	
	if (install_joystick(JOY_TYPE_AUTODETECT) == 0)
		configuration.joystick_installed = true;

	try {
		SetGraphicsMode();
	}
	catch (BadGraphicsMode) {
		allegro_message("Error setting graphics mode %dx%dx%d",
				configuration.screen_width,
				configuration.screen_height,
				configuration.color_depth);
		allegro_exit();
		exit(1);
	}

	try {
		InstallSound();
	}
	catch (BadInstall) {
	}

	for (int i = 0; i < NUM_DEFAULT_COLOR_SCHEMES; i++) {
		AddColorScheme(default_color_scheme_names[i], default_color_schemes[i]);
	}
	SetColorScheme(configuration.color_scheme);

	try {
		LoadData();
	}
	catch (BadLoad) {
		DestroyData();
		allegro_message("Error loading data");
		allegro_exit();
		exit(1);
	}

	try {
		CreateBuffer();
	}
	catch (std::bad_alloc) {
		allegro_message("Error creating buffer bitmap");
		allegro_exit();
		exit(1);
	}

	/*
	background_bitmap = create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
	if (!background_bitmap) {
		allegro_message("Error creating background bitmap");
		allegro_exit();
		exit(1);
	}
	*/

	GUI_Initialize();

	//DrawBackgroundBitmap();

	DrawLogo();

	int menu_choice;

	PlayIntroMusic();

	while ((menu_choice = StartMenu()) != START_MENU_EXIT) {
		switch (menu_choice) {
		case START_MENU_START_GAME:
			{
			bool again = true;
			while (again) {
				StopMusic();
				PlayGameMusic();
				bool esc_pressed = (*game_functions[configuration.game_type])();
				if (esc_pressed)
					again = false;
				else
					again = GUI_Prompt("|000255000P|000255255l|240000240a|255000000y |255255000a|000255000g|000255255a|240000240i|255000000n|255255000?", "Yes", "No");
				if (!again) {
					StopMusic();
					PlayIntroMusic();
				}
			}
			break;
			}
		case START_MENU_VIEW_HIGH_SCORES:
			ViewHighScores();
			break;
		case START_MENU_CONFIGURE:
			Configure();
			break;
		default:
			break;
		}
	}

	rest(200); // allow sample to finish playing

	WriteConfiguration(argv[0]);
	WriteHighScores(high_score_filename);

	DestroyData();
	allegro_exit();

	return 0;
}
END_OF_MAIN();

