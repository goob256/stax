#include <stdio.h>
#include <string.h>
#include <allegro.h>

#include "stax.h"

static BITMAP *bg;
static BITMAP *larrow = NULL, *larrow_d = NULL, *larrow_h = NULL;
static BITMAP *rarrow = NULL, *rarrow_d = NULL, *rarrow_h = NULL;
static SAMPLE *toggle = NULL;
static char **themes = NULL;
static int num_themes;

int toggle_proc(int, DIALOG *, int);
int shadow_text_proc(int, DIALOG *, int);
int text_button_proc(int, DIALOG *, int);
static int draw_screen(int, DIALOG *, int);
static int preview_theme(int, DIALOG *, int);
static char *get_style(int, int *);
static char *get_theme(int, int *);
static char *get_input_type(int, int *);
static char *get_sound(int, int *);

#define CFG_STYLE  2
#define CFG_THEME  4
#define CFG_INPUT1 6
#define CFG_INPUT2 8
#define CFG_SOUND  10
#define CFG_RETURN 11

DIALOG config_dialog[] =
{
	{ preview_theme,   155, 62, 50,50,      0, 0,     0,0,0,0,NULL,          NULL,NULL },
	{ shadow_text_proc, 50, 48, 80,15,C_WHITE, 0,     0,0,0,0,"Style",       NULL,NULL },
	{ toggle_proc,     100, 45,160,15,C_YELLOW,C_GREY,0,0,0,0,get_style,     NULL,NULL },
	{ shadow_text_proc, 50,118, 80,15,C_WHITE, 0,     0,0,0,0,"Theme",       NULL,NULL },
	{ toggle_proc,     100,115,160,15,C_YELLOW,C_GREY,0,0,0,0,get_theme,     NULL,NULL },
	{ shadow_text_proc, 50,133, 80,15,C_WHITE, 0,     0,0,0,0,"1P Input",    NULL,NULL },
	{ toggle_proc,     100,130,160,15,C_YELLOW,C_GREY,0,0,0,0,get_input_type,NULL,NULL },
	{ shadow_text_proc, 50,148, 80,15,C_WHITE, 0,     0,0,0,0,"2P Input",    NULL,NULL },
	{ toggle_proc,     100,145,160,15,C_YELLOW,C_GREY,0,0,0,0,get_input_type,NULL,NULL },
	{ shadow_text_proc, 50,163, 80,15,C_WHITE, 0,     0,0,0,0,"Sound",       NULL,NULL },
	{ toggle_proc,     100,160,160,15,C_YELLOW,C_GREY,0,0,0,0,get_sound,     NULL,NULL },
	{ text_button_proc,100,185, 64,15,C_YELLOW,C_GREY,0,0,0,0,"Return",      NULL,NULL },
	{ draw_screen,       0,  0,  0, 0,       0,0,     0,0,0,0,NULL,          NULL,NULL },
	{ NULL,              0,  0,  0, 0,       0,0,     0,0,0,0,NULL,          NULL,NULL }
};

/*
 * d1 = selected
 * d2 = number of items
 * dp = function pointer, returns item name char *func(int n)
 * fg = text color if selected
 * bg = text color if not selected
 */
int toggle_proc(int msg, DIALOG *d, int c)
{
	typedef char *(*getfuncptr)(int, int *);
	char *text;
	int x, y;
	int color;

	switch (msg) {
	case MSG_START:
		(*(getfuncptr)d->dp)(-1, &d->d2);
		break;
	case MSG_DRAW:
		if (d->flags & D_GOTFOCUS)
			color = d->fg;
		else
			color = d->bg;
		y = d->y + (d->h - larrow->h) / 2;
		text = (*(getfuncptr)d->dp)(d->d1, 0);
		if (d->d1)
			draw_sprite(vs, larrow, d->x, y);
		else
			draw_sprite(vs, larrow_d, d->x, y);
		if (d->d1 < d->d2-1)
			draw_sprite(vs, rarrow, d->x+d->w-rarrow->w, y);
		else
			draw_sprite(vs, rarrow_d, d->x+d->w-rarrow_d->w, y);
		x = d->x + (d->w - text_length(font, text)) / 2;
		y = d->y + (d->h - text_height(font)) / 2;
		text_mode(-1);
		textprintf_shadow(vs, font, x, y, color, C_BLACK, text);
		return D_REDRAW;
	case MSG_WANTFOCUS:
		return D_WANTFOCUS;
	case MSG_LOSTFOCUS:
		play_sample(toggle, 255, 128, 800, 0);
		break;
	case MSG_CHAR:
		if ((c >> 8) == KEY_LEFT) {
			if (d->d1) {
				d->d1--;
				y = d->y + (d->h - larrow_h->h) / 2;
				draw_sprite(screen, larrow_h, d->x, y);
				play_sample(toggle, 255, 0, 1000, 0);
				rest(100);
			}
		}
		else if ((c >> 8) == KEY_RIGHT) {
			if (d->d1 < d->d2-1) {
				d->d1++;
				y = d->y + (d->h - rarrow_h->h) / 2;
				draw_sprite(screen, rarrow_h, d->x+d->w-rarrow_h->w, y);
				play_sample(toggle, 255, 255, 1000, 0);
				rest(100);
			}
		}
		else
			break;
		scare_mouse();
		SEND_MESSAGE(d, MSG_DRAW, 0);
		unscare_mouse();
		return D_USED_CHAR | D_REDRAW;
	}

	return D_O_K;
}

int shadow_text_proc(int msg, DIALOG *d, int c)
{
	switch (msg) {
	case MSG_DRAW:
		text_mode(-1);
		textprintf_shadow(vs, font, d->x, d->y, d->fg, C_BLACK, d->dp);
		break;
	}

	return D_O_K;
}

int text_button_proc(int msg, DIALOG *d, int c)
{
	int color;

	switch (msg) {
	case MSG_DRAW:
		color = (d->flags & D_GOTFOCUS) ? d->fg : d->bg;
		text_mode(-1);
		textprintf_shadow(vs, font, d->x, d->y, color, C_BLACK, d->dp);
		break;
	case MSG_KEY:
		play_sample(toggle, 255, 128, 1000, 0);
		return D_CLOSE;
	case MSG_WANTFOCUS:
		return D_WANTFOCUS;
	case MSG_LOSTFOCUS:
		play_sample(toggle, 255, 128, 800, 0);
		break;
	}

	return D_O_K;
}

static int draw_screen(int msg, DIALOG *d, int c)
{
	switch (msg) {
	case MSG_START:
		blit(bg, vs, 0, 0, 0, 0, bg->w, bg->h);
		break;
	case MSG_DRAW:
		if (bg) {
			blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
			blit (bg, vs, 0, 0, 0, 0, bg->w, bg->h);
		}
		break;
	}

	return D_O_K;
}

static int preview_theme(int msg, DIALOG *d, int c)
{
	static int current_theme = 0;
	static BITMAP *preview = NULL;
	char theme_name[100];
	DATAFILE *datafile;

	if (msg == MSG_DRAW) {
		if (!preview || current_theme != config_dialog[CFG_THEME].d1) {
			current_theme = config_dialog[CFG_THEME].d1;
			snprintf(theme_name, sizeof(theme_name), "%s.dat", themes[current_theme]);
			datafile = load_datafile_object(theme_name, "PREVIEW");
			if (datafile)
				preview = datafile->dat;
		}
		if (preview)
			stretch_sprite(vs, preview, d->x, d->y, d->w, d->h);
	}

	return D_O_K;
}

static char *get_style(int i, int *num)
{
	char *styles[] = { "Sucka!", "Clobber", "Shifty", "Co-op", "Puzzle" };

	if (i < 0) {
		*num = 5;
		return NULL;
	}
	else
		return styles[i];
}

static char *get_theme(int i, int *num)
{
	if (i < 0) {
		*num = num_themes;
		return NULL;
	}
	else
		return themes[i];
}

static char *get_input_type(int i, int *num)
{
	char *inputs[] = {
		"Keys 1 (WASD/Space)", "Keys 2 (Arrows/Control)", "Joystick 1", "Joystick 2"
	};

	if (i < 0) {
		*num = 4;
		return NULL;
	}
	else
		return inputs[i];
}

static char *get_sound(int i, int *num)
{
	char *opts[] = { "ON", "OFF" };

	if (i < 0) {
		*num = 2;
		return NULL;
	}
	else {
		cfg.sound = !i;
		if (cfg.sound)
			set_volume(255, MIDI_VOLUME);
		else
			set_volume(0, 0);
		return opts[i];
	}
}

static void add_theme(char *filename, int attrib, int param)
{
	char theme_name[256] = { 0, };
	void *tmp;

	strncpy(theme_name, get_filename(filename), 256);
	*strrchr(theme_name, '.') = '\0';

	if (!strcasecmp(theme_name, "stax"))
		return;
	
	tmp = themes;
	themes = realloc(themes, ++num_themes * sizeof(char *));
	if (!themes) {
		themes = tmp;
		return;
	}

	themes[num_themes-1] = strdup(theme_name);
	if (themes[num_themes-1] == NULL) {
		themes = realloc(themes, --num_themes * sizeof(char *));
		return;
	}
}

void read_config(void)
{
	char buf[512];
	char keyword[512], value[512];
	FILE *f;

#ifdef DJGPP
	replace_filename(buf, argv0, "stax.cfg", sizeof(buf));
#else
	snprintf(buf, sizeof(buf), "%s/.staxrc", getenv("HOME"));
#endif

	f = fopen(buf, "r");
	if (!f)
		return;

	while (fgets(buf, sizeof(buf), f)) {
		if (buf[0] == '#')
			continue;
		if (sscanf(buf, "%s %s", keyword, value) == 2) {
			if (!strcasecmp(keyword, "style"))
				cfg.style = atoi(value);
			else if (!strcasecmp(keyword, "theme"))
				strncpy(cfg.theme, value, sizeof(cfg.theme));
			else if (!strcasecmp(keyword, "input1"))
				cfg.input1 = atoi(value);
			else if (!strcasecmp(keyword, "input2"))
				cfg.input2 = atoi(value);
			else if (!strcasecmp(keyword, "sound"))
				cfg.sound = atoi(value);
		}
	}

	fclose(f);
}

void write_config(void)
{
	FILE *f;
	char filename[512];

#ifdef DJGPP
	replace_filename(filename, argv0, "stax.cfg", sizeof(filename));
#else
	snprintf(filename, sizeof(filename), "%s/.staxrc", getenv("HOME"));
#endif

	f = fopen(filename, "w");
	if (!f)
		return;
	
	fprintf(f, 
		"# Style:\n"
		"#\n"
		"# 0 = Sucka!\n"
		"# 1 = Clobber\n"
		"# 2 = Shifty\n"
		"# 3 = Co-op\n"
		"# 4 = Puzzle\n\n"
	);
	fprintf(f, "style %d\n", cfg.style);
	fprintf(f, "\n");
	fprintf(f, "# Theme:\n\n");
	fprintf(f, "theme %s\n", cfg.theme);
	fprintf(f, "\n");
	fprintf(f,
		"# Input:\n"
		"#\n"
		"# 0 = Keys 1 (WASD/Space)\n"
		"# 1 = Keys 2 (Arrows/Control)\n"
		"# 2 = Joystick 1\n"
		"# 3 = Joystick 2\n\n"
	);
	fprintf(f, "input1 %d\n", cfg.input1);
	fprintf(f, "input2 %d\n", cfg.input2);
	fprintf(f, "\n");
	fprintf(f,
		"# Sound:\n"
		"#\n"
		"# 0 = Off\n"
		"# 1 = On\n\n"
	);
	fprintf(f, "sound %d\n", cfg.sound);
	
	fclose(f);
}

void config_loop(void)
{
	int i;

	larrow = find_object_data(dat, "LARROW");
	rarrow = find_object_data(dat, "RARROW");
	larrow_h = find_object_data(dat, "LARROW_H");
	rarrow_h = find_object_data(dat, "RARROW_H");
	larrow_d = find_object_data(dat, "LARROW_D");
	rarrow_d = find_object_data(dat, "RARROW_D");
	bg = find_object_data(dat, "CFG_BG");
	toggle = find_object_data(dat, "TOGGLE");

	read_config();

	num_themes = 0;
	for_each_file("*.dat", FA_ARCH | FA_RDONLY, add_theme, 0);

	if (num_themes == 0) {
		error("No themes were found! You can't play without em!", "Themes (*.dat) should be in the same directory as this binary", "Or in the directory STAX_DATA points to");
		return;
	}

	config_dialog[CFG_THEME].d1 = 0;
	for (i = 0; i < num_themes; i++)
		if (!strncasecmp(themes[i], cfg.theme, strlen(themes[i]))) {
			config_dialog[CFG_THEME].d1 = i;
			break;
		}
	config_dialog[CFG_STYLE].d1 = cfg.style;
	config_dialog[CFG_INPUT1].d1 = cfg.input1;
	config_dialog[CFG_INPUT2].d1 = cfg.input2;
	config_dialog[CFG_SOUND].d1 = !cfg.sound;

	focus_image(bg);

redo:
	do_dialog(config_dialog, CFG_STYLE);

	cfg.style = config_dialog[CFG_STYLE].d1;
	snprintf(cfg.theme, sizeof(cfg.theme), "%s.dat", themes[config_dialog[CFG_THEME].d1]);

	cfg.input1 = config_dialog[CFG_INPUT1].d1;
	cfg.input2 = config_dialog[CFG_INPUT2].d1;

	if (cfg.input1 == cfg.input2) {
		error("Both players can't use the same inputs!", "Choose something else for one player", NULL);
		goto redo;
	}

	if (num_themes) {
		do {
			free(themes[--num_themes]);
		} while (num_themes);
		free(themes);
		themes = NULL;
	}

	write_config();
}
