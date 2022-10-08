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

#include <vector>

#include "stax.h"

static std::vector<RGB*> color_schemes;
static std::vector<char*> color_scheme_names;

static char *DuplicateString(char *s) throw (std::bad_alloc)
{
	char *n = new char[strlen(s)+1];
	memcpy(n, s, strlen(s)+1);
	return n;
}

void AddColorScheme(char *name, RGB colors[NUM_CONFIGURABLE_COLORS]) throw (std::bad_alloc)
{
	RGB *scheme = new RGB[NUM_CONFIGURABLE_COLORS];
	for (int i = 0; i < NUM_CONFIGURABLE_COLORS; i++) {
		scheme[i].r = colors[i].r;
		scheme[i].g = colors[i].g;
		scheme[i].b = colors[i].b;
	}
	color_schemes.push_back(scheme);
	color_scheme_names.push_back(DuplicateString(name));
}

void SetColorScheme(char* name) throw (Invalid)
{
	for (unsigned int i = 0; i < color_schemes.size(); i++) {
		if (!strcmp(color_scheme_names[i], name)) {
			for (int j = 0; j < NUM_CONFIGURABLE_COLORS; j++) {
				configuration.colors[j].r = color_schemes[i][j].r;
				configuration.colors[j].g = color_schemes[i][j].g;
				configuration.colors[j].b = color_schemes[i][j].b;
			}
			return;
		}
	}
	throw Invalid();
}

int GetNumColorSchemes(void)
{
	return color_schemes.size();
}

char* GetColorSchemeName(unsigned int i) throw (Invalid)
{
	if (i >= color_scheme_names.size())
		throw Invalid();
	return color_scheme_names[i];
}

static char *game_type_getter(int n, int *size)
{
	char *strings[3] = {
		"Sucker", "SpringShot", "Shifty"
	};

	if (n < 0) {
		*size = 3;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static char *num_blocks_getter(int n, int *size)
{
	char *strings[4] = {
		"2", "3", "4", "5"
	};

	if (n < 0) {
		*size = 4;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static char *initial_height_getter(int n, int *size)
{
	char *strings[15] = {
		"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11",
		"12", "13", "14", "15"
	};

	if (n < 0) {
		*size = 15;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static void ConfigureGameOptions(void)
{
	Widget game_widgets[] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 140, 20, 0, 0, "Game Options", 0, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 60, 0, 0, "Game type:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT, 275, 60, 0, 0, NULL, 0, 0, 0, 0, (void *)game_type_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 85, 0, 0, "Initial height:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT, 275, 85, 0, 0, NULL, 0, 0, 0, 0, (void *)initial_height_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 110, 0, 0, "Block types:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT, 275, 110, 0, 0, NULL, 0, 0, 0, 0, (void *)num_blocks_getter, NULL, NULL, 0 },
		{ WIDGET_BUTTON, ALIGN_CENTER, 140, 140, -1, 0, "OK", 6, 7, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	game_widgets[0].d1 = makecol(255, 255, 0);

	game_widgets[2].d1 = (int)configuration.game_type;
	game_widgets[4].d1 = configuration.initial_height-1;
	game_widgets[6].d1 = configuration.number_of_block_types-2;

	int ret = GUI_Go(400-147, 300-97, 295, 195, game_widgets, 2, 0);

	configuration.game_type = (GameType)game_widgets[2].d1;
	configuration.initial_height = game_widgets[4].d1+1;
	configuration.number_of_block_types = game_widgets[6].d1+2;
}

static char *player1_controls_getter(int n, int *size)
{
	char *strings[4] = {
		"Arrows & Control", "W, A, S, D & Space", "Joystick 1", "Joystick 2"
	};

	if (n < 0) {
		*size = 4;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static char *player2_controls_getter(int n, int *size)
{
	char *strings[5] = {
		"None", "Arrows & Control", "W, A, S, D & Space", "Joystick 1", "Joystick 2"
	};

	if (n < 0) {
		*size = 5;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static void ConfigureControls(void)
{
	Widget control_widgets[] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 143, 20, 0, 0, "Controls", 0, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 60, 0, 0, "Player 1:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_CENTER, 183, 60, 0, 0, NULL, 0, 0, 0, 0, (void *)player1_controls_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 85, 0, 0, "Player 2:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_CENTER, 183, 85, 0, 0, NULL, 0, 0, 0, 0, (void *)player2_controls_getter, NULL, NULL, 0 },
		{ WIDGET_BUTTON, ALIGN_CENTER, 143, 115, -1, 0, "OK", 4, 5, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	control_widgets[0].d1 = makecol(255, 255, 0);

	control_widgets[2].d1 = (int)(configuration.input_type_1) - 1;
	control_widgets[4].d1 = (int)(configuration.input_type_2);

	int ret = 0;

	while (ret != 5 && ret >= 0) {
		ret = GUI_Go(400-143, 300-80, 286, 160, control_widgets, 2, 0);
		if (control_widgets[2].d1 == (control_widgets[4].d1-1)) {
			GUI_WaitMessage("|255000000Player 1 and Player 2 controls\n|255000000cannot be the same");
			ret = 0;
		}
	}

	configuration.input_type_1 = (InputType)(control_widgets[2].d1 + 1);
	configuration.input_type_2 = (InputType)(control_widgets[4].d1);
}

static char *graphics_mode_getter(int n, int *size)
{
	char *strings[2] = {
		"Windowed", "Full Screen"
	};

	if (n < 0) {
		*size = 2;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static char *resolution_getter(int n, int *size)
{
	char *strings[3] = {
		"640x480", "800x600", "1024x768"
	};

	if (n < 0) {
		*size = 3;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static char *color_depth_getter(int n, int *size)
{
	char *strings[4] = {
		"15", "16", "24", "32"
	};

	if (n < 0) {
		*size = 4;
		return NULL;
	}
	else {
		return strings[n];
	}
}

static void ConfigureGraphics(void)
{
	Widget graphics_widgets[] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 158, 20, 0, 0, "Graphics", 0, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 60, 0, 0, "Graphics mode:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT,287, 60, 0, 0, NULL, 0, 0, 0, 0, (void *)graphics_mode_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 85, 0, 0, "Resolution:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT, 287, 85, 0, 0, NULL, 0, 0, 0, 0, (void *)resolution_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 110, 0, 0, "Color depth:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_RIGHT, 287, 110, 0, 0, NULL, 0, 0, 0, 0, (void *)color_depth_getter, NULL, NULL, 0 },
		{ WIDGET_BUTTON, ALIGN_CENTER, 158, 140, -1, 0, "OK", 2, 3, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	graphics_widgets[0].d1 = makecol(255, 255, 0);

	int old_driver = configuration.graphics_driver;
	int old_depth = configuration.color_depth;
	int old_width = configuration.screen_width;
	int old_height = configuration.screen_height;

	if (old_driver == GFX_AUTODETECT_FULLSCREEN)
		graphics_widgets[2].d1 = 1;
	else
		graphics_widgets[2].d1 = 0;

	if (configuration.screen_width == 640 && configuration.screen_height == 480)
		graphics_widgets[4].d1 = 0;
	else if (configuration.screen_width == 1024 && configuration.screen_height == 768)
		graphics_widgets[4].d1 = 2;
	else
		graphics_widgets[4].d1 = 1;

	switch (configuration.color_depth) {
		case 32:
			graphics_widgets[6].d1 = 3;
			break;
		case 24:
			graphics_widgets[6].d1 = 2;
			break;
		case 16:
			graphics_widgets[6].d1 = 1;
			break;
		case 15:
			graphics_widgets[6].d1 = 0;
			break;
	}

	GUI_Go(400-158, 300-97, 316, 195, graphics_widgets, 2, 0);

	if (graphics_widgets[2].d1 == 0)
		configuration.graphics_driver = GFX_AUTODETECT_WINDOWED;
	else
		configuration.graphics_driver = GFX_AUTODETECT_FULLSCREEN;

	switch (graphics_widgets[4].d1) {
		case 0:
			configuration.screen_width = 640;
			configuration.screen_height = 480;
			break;
		case 1:
			configuration.screen_width = 800;
			configuration.screen_height = 600;
			break;
		case 2:
			configuration.screen_width = 1024;
			configuration.screen_height = 768;
			break;
	}

	switch (graphics_widgets[6].d1) {
		case 0:
			configuration.color_depth = 15;
			break;
		case 1:
			configuration.color_depth = 16;
			break;
		case 2:
			configuration.color_depth = 24;
			break;
		case 3:
			configuration.color_depth = 32;
			break;
	}

	try {
		SetGraphicsMode();
		DestroyBuffer();
		CreateBuffer();
		DestroyData();
		LoadData();
		GUI_Initialize();
	}
	catch (BadGraphicsMode) {
		configuration.graphics_driver = old_driver;
		configuration.screen_width = old_width;
		configuration.screen_height = old_height;
		configuration.color_depth = old_depth;
		SetGraphicsMode();
		GUI_WaitMessage("|255000000Error setting graphics mode...\n|255000000Reverted to previous mode.");
	}
	catch (...) {
		allegro_message("Error reloading data");
		allegro_exit();
		exit(1);
	}
	PlayIntroMusic();
}

struct SoundDriver {
	int driver;
	char* name;
};

#ifdef __linux__
const int num_digi_drivers = 7;
static SoundDriver digi_drivers[] = {
	{ DIGI_NONE, "None" },
	{ DIGI_AUTODETECT, "Autodetect" },
	{ DIGI_ALSA, "ALSA" },
	{ DIGI_OSS, "OSS" },
	{ DIGI_ESD, "ESD" },
	{ DIGI_ARTS, "aRts" },
	{ DIGI_JACK, "JACK" },
};

const int num_midi_drivers = 5;
static SoundDriver midi_drivers[] = {
	{ MIDI_NONE, "None" },
	{ MIDI_AUTODETECT, "Autodetect" },
	{ MIDI_OSS, "OSS" },
	{ MIDI_ALSA, "ALSA" },
	{ MIDI_DIGMID, "DIGMID" }
};
#else // Windows
const int num_digi_drivers = 2;
static SoundDriver digi_drivers[] = {
	{ DIGI_NONE, "None" },
	{ DIGI_AUTODETECT, "Autodetect" }
};

const int num_midi_drivers = 3;
static SoundDriver midi_drivers[] = {
	{ MIDI_NONE, "None" },
	{ MIDI_AUTODETECT, "Autodetect" },
	{ MIDI_DIGMID, "DIGMID" }
};
#endif

static char *digi_getter(int n, int *size)
{
	if (n < 0) {
		*size = num_digi_drivers;
		return NULL;
	}
	else {
		return digi_drivers[n].name;
	}
}

static char *midi_getter(int n, int *size)
{
	if (n < 0) {
		*size = num_midi_drivers;
		return NULL;
	}
	else {
		return midi_drivers[n].name;
	}
}

static void ConfigureSound(void)
{
	Widget sound_widgets[] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 213, 20, 0, 0, "Sound", 0, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 60, 0, 0, "Audio Device:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_LEFT, 119, 60, 0, 0, NULL, 0, 0, 0, 0, (void *)digi_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 85, 0, 0, "Music Device:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_OPTION, ALIGN_LEFT, 119, 85, 0, 0, NULL, 0, 0, 0, 0, (void *)midi_getter, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 110, 0, 0, "Audio volume:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_SLIDER, ALIGN_LEFT, 119, 110, 255, 0, 0, 0, 1, 10, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_LEFT, 20, 135, 0, 0, "Music volume:", -1, 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_SLIDER, ALIGN_LEFT, 119, 135, 255, 0, 0, 0, 1, 10, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_BUTTON, ALIGN_CENTER, 213, 165, -1, 0, "OK", 8, 9, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	sound_widgets[0].d1 = makecol(255, 255, 0);

	sound_widgets[2].d1 = 0;
	for (int i = 0; i < num_digi_drivers; i++)
		if (digi_drivers[i].driver == configuration.sound_driver) {
			sound_widgets[2].d1 = i;
			break;
		}

	sound_widgets[4].d1 = 0;
	for (int i = 0; i < num_midi_drivers; i++)
		if (midi_drivers[i].driver == configuration.midi_driver) {
			sound_widgets[4].d1 = i;
			break;
		}

	sound_widgets[6].d1 = configuration.sound_volume;
	sound_widgets[8].d1 = configuration.music_volume;
	
	GUI_Go(400-213, 300-110, 427, 220, sound_widgets, 2, 0);

	rest(200); // allow sample to finish playing

	configuration.sound_driver = digi_drivers[sound_widgets[2].d1].driver;
	configuration.midi_driver = midi_drivers[sound_widgets[4].d1].driver;
	configuration.sound_volume = sound_widgets[6].d1;
	configuration.music_volume = sound_widgets[8].d1;

	for (;;) {
		RemoveSound();
		try {
			InstallSound();
		}
		catch (BadInstall) {
			if (GUI_Prompt("|255000000Error setting up sound... Try again?", "Yes", "No") == 1)
				continue;
		}
		break;
	}
	PlayIntroMusic();
}

void Configure(void)
{
	clear(buffer);

	Widget configure_widgets[] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 95, 20, 0, 0, "Configuration", 0, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 95, 60, 0, 0, "Game Options", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 95, 80, 0, 0, "Controls", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 95, 100, 0, 0, "Graphics", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXTBUTTON, ALIGN_CENTER, 95, 120, 0, 0, "Sound", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_BUTTON, ALIGN_CENTER, 95, 160, -1, 0, "OK", -1, -1, -1, -1, NULL, NULL, NULL, 0 },
		{ WIDGET_END, }
	};

	int ret = 0;
	while (ret != 5 && ret >= 0) {
		configure_widgets[0].d1 = makecol(255, 255, 0);
		ret = GUI_Go(400-95, 300-105, 190, 210, configure_widgets, 1, 0);

		switch (ret) {
		case 1: ConfigureGameOptions(); break;
		case 2: ConfigureControls(); break;
		case 3: ConfigureGraphics(); break;
		case 4: ConfigureSound(); break;
		default: break;
		}
	}
}

void ReadConfiguration(char* argv0)
{
	char buf[512];
	char keyword[512], value[512];
	FILE *f;
	int v;

#ifdef __linux__ 
	snprintf(buf, sizeof(buf), "%s/.staxrc", getenv("HOME"));
#else
	replace_filename(buf, argv0, "stax.cfg", sizeof(buf));
#endif

	f = fopen(buf, "r");
	if (!f)
		return;

	while (fgets(buf, sizeof(buf), f)) {
		if (buf[0] == '#')
			continue;
		if (sscanf(buf, "%s %s", keyword, value) == 2) {
			v = atoi(value);
			if (!strcasecmp(keyword, "game")) {
				if (v < 0 || v >= NUMBER_OF_GAME_TYPES)
					v = 0;
				configuration.game_type = (GameType)v;
			}
			else if (!strcasecmp(keyword, "input1")) {
				if (v <= 0 || v >= NUMBER_OF_INPUT_TYPES)
					v = 1;
				configuration.input_type_1 = (InputType)v;
			}
			else if (!strcasecmp(keyword, "input2")) {
				if (v < 0 || v >= NUMBER_OF_INPUT_TYPES)
					v = 0;
				configuration.input_type_2 = (InputType)v;
			}
			else if (!strcasecmp(keyword, "blocks")) {
				if (v < 2 || v > 5)
					v = 4;
				configuration.number_of_block_types = v;
			}
			else if (!strcasecmp(keyword, "height")) {
				if (v < 1 || v > 15)
					v = 5;
				configuration.initial_height = v;
			}
			else if (!strcasecmp(keyword, "mode")) {
				if (v < 0 || v > 1)
					v = 0;
				if (v == 0)
					configuration.graphics_driver = GFX_AUTODETECT_WINDOWED;
				else
					configuration.graphics_driver = GFX_AUTODETECT_FULLSCREEN;
			}
			else if (!strcasecmp(keyword, "digi_driver")) {
				configuration.sound_driver = v;
			}
			else if (!strcasecmp(keyword, "midi_driver")) {
				configuration.midi_driver = v;
			}
			else if (!strcasecmp(keyword, "digi_volume")) {
				if (v < 0 || v > 255)
					v = 255;
				configuration.sound_volume = v;
			}
			else if (!strcasecmp(keyword, "midi_volume")) {
				if (v < 0 || v > 255)
					v = 255;
				configuration.music_volume = v;
			}
			else if (!strcasecmp(keyword, "depth")) {
				if (v != 24 && v != 16 && v != 15)
					v = 32;
				configuration.color_depth = v;
			}
			else if (!strcasecmp(keyword, "sw")) {
				configuration.screen_width = v;
			}
			else if (!strcasecmp(keyword, "sh")) {
				configuration.screen_height = v;
			}
		}
	}

	if (configuration.input_type_1 == configuration.input_type_2) {
		configuration.input_type_1 = INPUT_RIGHT_KEYBOARD;
		configuration.input_type_2 = INPUT_NONE;
	}

	fclose(f);
}

void WriteConfiguration(char* argv0)
{
	FILE *f;
	char filename[512];

#ifdef __linux__ 
	snprintf(filename, sizeof(filename), "%s/.staxrc", getenv("HOME"));
#else
	replace_filename(filename, argv0, "stax.cfg", sizeof(filename));
#endif

	f = fopen(filename, "w");
	if (!f)
		return;
	
	fprintf(f, 
		"# Default Game Type:\n"
		"#\n"
		"# 0 = Sucker\n"
		"# 1 = SpringShot\n"
		"# 2 = Shifty\n"
	);
	fprintf(f, "game %d\n", (int)configuration.game_type);
	fprintf(f, "\n");
	fprintf(f,
		"# Input (Player 1 and 2):\n"
		"#\n"
		"# 0 = Keys 1 (Arrows)\n"
		"# 1 = Keys 2 (WASD)\n"
		"# 2 = Joystick 1\n"
		"# 3 = Joystick 2\n\n"
	);
	fprintf(f, "input1 %d\n", (int)configuration.input_type_1);
	fprintf(f, "input2 %d\n", (int)configuration.input_type_2);
	fprintf(f, "\n");
	fprintf(f, "# Number of different block types\n");
	fprintf(f, "blocks %d\n", configuration.number_of_block_types);
	fprintf(f, "\n");
	fprintf(f, "# Initial height of the blocks\n");
	fprintf(f, "height %d\n", configuration.initial_height);
	fprintf(f, "\n");
	fprintf(f, 
			"# Graphics mode:\n"
			"#\n"
			"# 0 = Windowed\n"
			"# 1 = Full screen\n"
			"\n"
	);
	int mode;
	if (configuration.graphics_driver == GFX_AUTODETECT_WINDOWED)
		mode = 0;
	else
		mode = 1;
	fprintf(f, "mode %d\n", mode);
	fprintf(f, "\n");
	fprintf(f, "# Sound driver (system dependant, see Allegro docs)\n");
	fprintf(f, "digi_driver %d\n", configuration.sound_driver);
	fprintf(f, "# MIDI driver (system dependant, see Allegro docs)\n");
	fprintf(f, "midi_driver %d\n", configuration.midi_driver);
	fprintf(f, "# Sound volume\n");
	fprintf(f, "digi_volume %d\n", configuration.sound_volume);
	fprintf(f, "# MIDI volume\n");
	fprintf(f, "midi_volume %d\n", configuration.music_volume);
	fprintf(f, "# Color depth\n");
	fprintf(f, "depth %d\n", configuration.color_depth);
	fprintf(f, "# Screen width\n");
	fprintf(f, "sw %d\n", configuration.screen_width);
	fprintf(f, "# Screen height\n");
	fprintf(f, "sh %d\n", configuration.screen_height);
	
	fclose(f);
}
