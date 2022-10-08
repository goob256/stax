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

#include "stax.h"

// FIXME: Some of these should throw exceptions

char* GetHighScoresFilename(char* argv0)
{
	static char filename[1000];
	char *env = getenv("STAX_HIGH_SCORES");

	if (env) {
		strncpy(filename, env, sizeof(filename));
	}
	else {
#ifdef __linux__
		env = getenv("HOME");
		if (env) {
			strncpy(filename, env, sizeof(filename));
			if (filename[strlen(filename)-1] != '/')
				strncat(filename, "/", sizeof(filename));
			strncat(filename, ".staxhs", sizeof(filename));
		}
		else {
			replace_filename(filename, argv0, "stax.hs", sizeof(filename));
		}
#else
		replace_filename(filename, argv0, "stax.hs", sizeof(filename));
#endif
	}

	return filename;
}

static int iputl(long l, FILE *f)
{
	if (fputc(l & 0xFF, f) == EOF) {
		return -1;
	}
	if (fputc((l >> 8) & 0xFF, f) == EOF) {
		return -1;
	}
	if (fputc((l >> 16) & 0xFF, f) == EOF) {
		return -1;
	}
	if (fputc((l >> 24) & 0xFF, f) == EOF) {
		return -1;
	}
	return 0;
}

static long igetl(FILE *f)
{
	int c1 = fgetc(f);
	int c2 = fgetc(f);
	int c3 = fgetc(f);
	int c4 = fgetc(f);
	return (long)c1 | ((long)c2 << 8) | ((long)c3 << 16) | ((long)c4 << 24);
}

static void ReadName(char *buffer, FILE *f)
{
	int i;
	int c;
	
	for (i = 0; i < MAX_NAME; i++) {
		c = fgetc(f);
		if (c == EOF || c == '\0')
			break;
		buffer[i] = c;
	}
	buffer[i] = '\0';
}

static int WriteName(char *name, FILE *f)
{
	int i;

	for (i = 0; name[i]; i++) {
		if (fputc(name[i], f) == EOF)
			return -1;
	}
	if (fputc('\0', f) == EOF)
		return -1;
	return 0;
}

int ReadHighScores(char *filename)
{
	int i, j;
	FILE *f = fopen(filename, "r");
	if (!f)
		return -1;

	for (j = 0; j < NUMBER_OF_GAME_TYPES; j++) {
		for (i = 0; i < NUM_HIGH_SCORES; i++) {
			ReadName(high_scores[j][i].name, f);
			high_scores[j][i].score = igetl(f);
			high_scores[j][i].blocks = igetl(f);
		}
	}
	return 0;
}

int WriteHighScores(char *filename)
{
	int i, j;
	FILE *f = fopen(filename, "w");
	if (!f)
		return -1;

	for (j = 0; j < NUMBER_OF_GAME_TYPES; j++) {
		for (i = 0; i < NUM_HIGH_SCORES; i++) {
			if (WriteName(high_scores[j][i].name, f) < 0)
				return -1;
			iputl(high_scores[j][i].score, f);
			iputl(high_scores[j][i].blocks, f);
		}
	}
	return 0;
}

/*
 * Check if the score belongs in the high scores list,
 * and if so, prompt the player for their name.
 */
void CheckHighScores(int pops)
{
	if (pops <= 0)
		return;

	int i;
	int j;
	int game_type = (int)configuration.game_type;

	for (i = 0; i < NUM_HIGH_SCORES; i++) {
		if (pops >= high_scores[game_type][i].score)
			break;
	}

	if (i == NUM_HIGH_SCORES)
		return;

	for (j = NUM_HIGH_SCORES-1; j > i; j--) {
		strcpy(high_scores[game_type][j].name, high_scores[game_type][j-1].name);
		high_scores[game_type][j].score = high_scores[game_type][j-1].score;
		high_scores[game_type][j].blocks = high_scores[game_type][j-1].blocks;
	}

	high_scores[game_type][i].score = pops;
	high_scores[game_type][i].blocks = configuration.number_of_block_types;

	Widget widgets[23] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 178, 20, 0, 0, "You made the High Score list!", makecol(255, 255, 0), 0, 0, 0, NULL, NULL, NULL, 0 },
		{ WIDGET_TEXT, ALIGN_CENTER, 178, 40, 0, 0, "Enter your name:", makecol(255, 255, 0), 0, 0, 0, NULL, NULL, NULL, 0 },
	};
	widgets[22].type = WIDGET_END;

	int y = 80;
	char buffer[21] = "";
	char score_text[NUM_HIGH_SCORES][30];

	for (j = 0; j < NUM_HIGH_SCORES; j++) {
		sprintf(score_text[j], "%d (%d)", high_scores[game_type][j].score, high_scores[game_type][j].blocks);
	}

	for (j = 0; j < NUM_HIGH_SCORES; j++) {
		int ni = 2 + j;
		int si = ni + NUM_HIGH_SCORES;
		if (j == i) {
			widgets[ni].type = WIDGET_EDITOR;
			widgets[ni].align = ALIGN_LEFT;
			widgets[ni].s = buffer;
			widgets[ni].d1 = 20;
			widgets[ni].d2 = 1;
			widgets[ni].d3 = VALID_NAME;
			widgets[ni].d4 = -1;
			widgets[ni].x = 20;
			widgets[ni].y = y;
		}
		else {
			widgets[ni].type = WIDGET_TEXT;
			widgets[ni].align = ALIGN_LEFT;
			if (j % 2)
				widgets[ni].d1 = makecol(150, 150, 150);
			else
				widgets[ni].d1 = makecol(255, 255, 255);
			widgets[ni].x = 20;
			widgets[ni].y = y;
			widgets[ni].s = high_scores[game_type][j].name;
		}
		widgets[si].type = WIDGET_TEXT;
		widgets[si].align = ALIGN_RIGHT;
		if (j % 2)
			widgets[si].d1 = makecol(150, 150, 150);
		else
			widgets[si].d1 = makecol(255, 255, 255);
		widgets[si].x = 336;
		if (j == i)
			widgets[si].y = y + 3;
		else
			widgets[si].y = y;
		widgets[si].s = score_text[j];
		if (j == i)
			y += 30;
		else
			y += 25;
	}
	
	GUI_Go(400-(356/2), 300-(355/2), 356, 355, widgets, i+2, 0);

	if (!strcmp(buffer, ""))
		strcpy(buffer, "Anonymous");

	strcpy(high_scores[game_type][i].name, buffer);
}

void ViewHighScores(void)
{
	int game_type = 0;
	char* game_text[] = { "Sucker High Scores", "SpringShot High Scores",
		"Shifty High Scores" };

	Widget widgets[25] = {
		{ WIDGET_TEXT, ALIGN_CENTER, 178, 20, 0, 0, game_text[0], -1, 0, 0, 0, NULL, NULL, NULL, 0 },
	};
	widgets[24].type = WIDGET_END;

	int y = 60 + (25*NUM_HIGH_SCORES) + 10;
	widgets[21].type = WIDGET_BUTTON;
	widgets[21].align = ALIGN_LEFT;
	widgets[21].x = 52;
	widgets[21].y = y;
	widgets[21].w = -1;
	widgets[21].s = "<";
	widgets[21].d1 = 21;
	widgets[21].d2 = 22;
	widgets[22].type = WIDGET_BUTTON;
	widgets[22].align = ALIGN_LEFT;
	widgets[22].x = 134;
	widgets[22].y = y;
	widgets[22].w = -1;
	widgets[22].s = "Exit";
	widgets[22].d1 = 21;
	widgets[22].d2 = 23;
	widgets[23].type = WIDGET_BUTTON;
	widgets[23].align = ALIGN_LEFT;
	widgets[23].x = 232;
	widgets[23].y = y;
	widgets[23].w = -1;
	widgets[23].s = ">";
	widgets[23].d1 = 22;
	widgets[23].d2 = 23;

	int ret = 22;

	for (;;) {
		y = 60;
		char buffer[21] = "";
		char score_text[NUM_HIGH_SCORES][30];
		int j;

		for (j = 0; j < NUM_HIGH_SCORES; j++) {
			sprintf(score_text[j], "%d (%d)", high_scores[game_type][j].score, high_scores[game_type][j].blocks);
		}

		for (j = 0; j < NUM_HIGH_SCORES; j++) {
			int ni = 1 + j;
			int si = ni + NUM_HIGH_SCORES;
			widgets[ni].type = WIDGET_TEXT;
			widgets[ni].align = ALIGN_LEFT;
			if (j % 2)
				widgets[ni].d1 = makecol(150, 150, 150);
			else
				widgets[ni].d1 = makecol(255, 255, 255);
			widgets[ni].x = 20;
			widgets[ni].y = y;
			widgets[ni].s = high_scores[game_type][j].name;
			widgets[si].type = WIDGET_TEXT;
			widgets[si].align = ALIGN_RIGHT;
			if (j % 2)
				widgets[si].d1 = makecol(150, 150, 150);
			else
				widgets[si].d1 = makecol(255, 255, 255);
			widgets[si].x = 336;
			widgets[si].y = y;
			widgets[si].s = score_text[j];
			y += 25;
		}
		ret = GUI_Go(400-(366/2), 300-(375/2), 366, 375, widgets, ret, 0);
		if (ret == 21) {
			game_type--;
			if (game_type < 0)
				game_type = NUMBER_OF_GAME_TYPES-1;
		}
		else if (ret == 23) {
			game_type++;
			if (game_type >= NUMBER_OF_GAME_TYPES)
				game_type = 0;
		}
		else
			break;
		widgets[0].s = game_text[game_type];
	}
}
