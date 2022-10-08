/*
 * Code shared between all block games
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>

#include "stax.h"

/*
 * Returns the datafile path with filename appended to it
 */
char *datapath(char *filename)
{
	static char path[512];
	char *env = getenv("STAX_DATA");

	if (env)
		strncpy(path, env, sizeof(path));
	else
		replace_filename(path, argv0, "", sizeof(path));

	if (path[strlen(path)-1] != '/')
		strncat(path, "/", sizeof(path));

	strncat(path, filename, sizeof(path));

	return path;
}

/*
 * Searches through a datafile for the object name passed in
 * printf style arguments, and returns its data. It returns
 * NULL if not found.
 */
void *find_object_data(DATAFILE *d, char *fmt, ...)
{
	va_list ap;
	char name[512];
	int i;

	va_start(ap, fmt);
	vsnprintf(name, sizeof(name), fmt, ap);
	va_end(ap);

	for (i = 0; d[i].type != DAT_END; i++) {
		if (!strcmp(get_datafile_property(d+i, DAT_ID('N','A','M','E')),name))
			return d[i].dat;
	}

	return NULL;
}

/* print an error message up to 3 lines and wait for key/joystick button */
void error(char *line1, char *line2, char *line3)
{
	BITMAP *bg = find_object_data(dat, "ERROR");
	SAMPLE *sample = find_object_data(dat, "CUCKOO");

	play_sample(sample, 255, 128, 1000, 0);

	blit(bg, screen, 0, 0, 0, 0, bg->w, bg->h);

	text_mode(-1);

	if (line1)
		textprintf_shadow(screen, font, (SCREEN_W-text_length(font, line1))/2, 80, C_WHITE, C_BLACK, line1);
	if (line2)
		textprintf_shadow(screen, font, (SCREEN_W-text_length(font, line2))/2, 100, C_WHITE, C_BLACK, line2);
	if (line3)
		textprintf_shadow(screen, font, (SCREEN_W-text_length(font, line3))/2, 120, C_WHITE, C_BLACK, line3);

	clear_keybuf();
	joy_b1 = joy_b2 = joy_b3 = joy_b4 = 0;
	
	do {
		poll_joystick();
	} while (!keypressed() && !joy_b1 && !joy_b2 && !joy_b3 && !joy_b4);
	if (keypressed())
		readkey();
}

/* like textprintf, only with a shadow offset below the text */
void textprintf_shadow(BITMAP *bmp, FONT *f, int x, int y,
	int text_color, int shadow_color, char *format, ...)
{
   char buf[512];

   va_list ap;
   va_start(ap, format);
   uvsprintf(buf, format, ap);
   va_end(ap);

   textout(bmp, f, buf, x, y+1, shadow_color);
   textout(bmp, f, buf, x, y, text_color);
}


int prompt(char *s, char *r1, char *r2)
{
	int selected = 0;
	int x = (SCREEN_W - 10 - text_length(font, r1) - text_length(font, r2)) / 2;
	int y = (SCREEN_H - (text_height(font) * 2) - 5) / 2;
	SAMPLE *toggle = find_object_data(dat, "TOGGLE");
	
	rectfill(screen, (SCREEN_W/2)-50, (SCREEN_H/2)-20, (SCREEN_W/2)+50, (SCREEN_H/2)+20, C_BLACK);
	rect(screen, (SCREEN_W/2)-50, (SCREEN_H/2)-20, (SCREEN_W/2)+50, (SCREEN_H/2)+20, C_WHITE);
	rect(screen, (SCREEN_W/2)-49, (SCREEN_H/2)-19, (SCREEN_W/2)+49, (SCREEN_H/2)+19, C_GREY);
	rect(screen, (SCREEN_W/2)-48, (SCREEN_H/2)-18, (SCREEN_W/2)+48, (SCREEN_H/2)+18, C_DGREY);
	
	text_mode(-1);

	while (1) {
		textout_centre(screen, font, s, SCREEN_W / 2, y, C_WHITE);
		if (selected == 0) {
			textout_centre(screen, font, r1, x+5, y+text_height(font)+5, C_YELLOW);
			textout_centre(screen, font, r2, x+text_length(font, r1)+10, y+text_height(font)+5, C_GREY);
		}
		else {
			textout_centre(screen, font, r1, x+5, y+text_height(font)+5, C_GREY);
			textout_centre(screen, font, r2, x+text_length(font, r1)+10, y+text_height(font)+5, C_YELLOW);
		}
		poll_keyboard();
		poll_joystick();
		if (key[KEY_LEFT] || key[KEY_RIGHT] || joy_left || joy_right) {
			selected = !selected;
			play_sample(toggle, 255, 128, 1500, 0);
			rest(200);
		}
		else if (key[KEY_ENTER] || key[KEY_SPACE] || joy_b1 || joy_b2 || joy_b3 || joy_b4) {
			play_sample(toggle, 255, 128, 500, 0);
			rest(500);
			break;
		}
	}

	return selected;
}

void do_pause(void)
{
	SAMPLE *samp = find_object_data(dat, "PAUSE");

	play_sample(samp, 255, 128, 1000, 0);

	text_mode(-1);
	textprintf_shadow(screen, font, (SCREEN_W-text_length(font, "PAUSED"))/2, SCREEN_H/2, C_WHITE, C_BLACK, "PAUSED");

	set_volume(255, MIDI_VOLUME/2);
	clear_keybuf();
	rest(200);

	do {
		poll_joystick();
	} while (!keypressed() && !joy_b1 && !joy_b2 && !joy_b3 && !joy_b4);

	play_sample(samp, 255, 128, 800, 0);
	
	set_volume(255, MIDI_VOLUME);
}

#define MAX_SCREENSHOTS  1000

void screenshot(void)
{
	int i;
	char filename[20];

	for (i = 0; i < MAX_SCREENSHOTS; i++) {
		snprintf(filename, sizeof(filename), "stax%04d.pcx", i);
		if (!file_exists(filename, 0, NULL)) {
			save_pcx(filename, vs, palette);
			break;
		}
	}

	rest(500);
}

int game_over(Panel *winner, Panel *loser, Theme *t)
{
	int ret;

	set_volume(255, MIDI_VOLUME/2);

	winner->player_bitmap = PLAYER_WIN;
	winner->player_count = ~0;
	loser->player_bitmap = PLAYER_LOSE;
	loser->player_count = ~0;

	blit(t->background[t->background_frame], vs, 0, 0, 0, 0, t->background[t->background_frame]->w, t->background[t->background_frame]->h);
	draw_panel(winner, t);
	draw_panel(loser, t);

	if (winner->player_number == 0) {
		draw_sprite(vs, t->player_bitmaps[0][winner->player_bitmap], 0, 0);
		draw_sprite(vs, t->player_bitmaps[1][loser->player_bitmap], vs->w - t->player_bitmaps[1][loser->player_bitmap]->w, 0);
	}
	else {
		draw_sprite(vs, t->player_bitmaps[0][loser->player_bitmap], 0, 0);
		draw_sprite(vs, t->player_bitmaps[1][winner->player_bitmap], vs->w - t->player_bitmaps[1][winner->player_bitmap]->w, 0);
	}

	blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);

	play_sample(t->player_samples[winner->player_number][PLAYER_WIN], 255, 128, 1000, 0);
	rest(200);
	play_sample(t->player_samples[loser->player_number][PLAYER_LOSE], 180, 128, 1000, 0);

	rest(2000);

	ret = prompt("Play Again?", "Yes", "No");

	stop_midi();
	set_volume(255, MIDI_VOLUME);

	return ret;
}

/*
 * Zooms an image onto the screen, with a little wobbly bounce effect
 */
void zoom_image(BITMAP *b)
{
	int n, percent = 140, inc;
	int w, h;
	fixed current = 0;

	for (n = 0; n < 6; n++) {
		switch (n) {
		case 1: percent = 80; break;
		case 2: percent = 110; break;
		case 3: percent = 95; break;
		case 4: percent = 105; break;
		case 5: percent = 100; break;
		}
		inc = (current>>16) < percent ? 1 : -1;
		while (1) {
			if (inc > 0 && (current>>16) >= percent)
				break;
			else if (inc < 0 && (current>>16) <= percent)
				break;
			tick = 0;
			w = b->w * (current>>16) / 100;
			h = b->h * (current>>16) / 100;
			clear(vs);
			stretch_blit(b, vs, 0, 0, b->w, b->h, (vs->w-w)/2, (vs->h-h)/2, w, h);
			blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
			current += (inc > 0) ?
				fmul(ftofix(0.2), itofix(tick)) : -fmul(ftofix(0.2), itofix(tick));
		}
	}
}

/*
 * Puts the bitmap on screen with a blocky focus effect
 */
void focus_image(BITMAP *b)
{
	int size = 64;
	int x, y, color;

	do {
		clear(vs);
		for (y = 0; y < b->h; y += size)
			for (x = 0; x < b->w; x += size) {
				color = getpixel(b, x, y);
				rectfill(vs, x, y, x+size, y+size, color);
			}
		blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
		rest(50);
		size /= 2;
	} while (size);
}

/*
 * Add a block to the hash table, sorted, with given data
 */
int block_add(Block *hash[PANEL_W], int x, fixed y, int type)
{
	Block *new = calloc(1, sizeof(Block));
	Block *tmp = hash[x], *prev = NULL;

	if (!new)
		return 0;

	new->x = x;
	new->y = y;
	new->type = type;

	if (hash[x] == NULL) {
		hash[x] = new;
		return 1;
	}
	
	while (tmp && tmp->y < y)
		prev = tmp, tmp = tmp->next;
	
	if (!tmp) {
		prev->next = new;
		new->prev = prev;
		new->next = NULL;
	}
	else if (!tmp->prev) {
		new->next = hash[x];
		hash[x]->prev = new;
		hash[x] = new;
	}
	else {
		tmp->prev->next = new;
		new->prev = tmp->prev;
		new->next = tmp;
		tmp->prev = new;
	}

	return 1;
}

/* delete a block from the hash */
void block_delete(Block *hash[PANEL_W], Block *node)
{
	Block *tmp = hash[node->x];

	while (tmp && tmp != node)
		tmp = tmp->next;
	
	if (!tmp)
		return;
	
	if (tmp->prev)
		tmp->prev->next = tmp->next;
	else
		hash[node->x] = tmp->next;
	if (tmp->next)
		tmp->next->prev = tmp->prev;

	free(node);
}

/* 
 * make blocks without anything under them fall, and the bottom
 * row rise gradually
 *
 * return -1 on overflow, 0 if nothing moved and 1 if something moved
 */
int move_blocks(Panel *p, Theme *t, int timer_tick)
{
	Block *b;
	int x, ret = 0;
	int colliding_blocks = 0;
	int gen_bottom_row = 0;
	fixed tmp;

	/* check for popping blocks and remove them if time */
	if (p->blocks_popping) {
		p->flash_count -= timer_tick;
		if (p->flash_count <= 0) {
			p->type_flashing = !p->type_flashing;
			p->flash_count = FLASH_DURATION;
		}
		if (p->pop_count <= 0) {
			int played = 0;
			for (x = 0; x < PANEL_W; x++) {
				for (b = p->blocks[x]; b; b = b->next) {
					if (b->popping) {
						if (b->prev)
							b->prev->glued = 0;
						block_delete(p->blocks, b);
						if (!played) {
							play_sample(t->pop, 255, 128, 1000, 0);
							played = 1;
						}
					}
				}
			}
			p->blocks_popping = 0;
			p->pop_count = 0;
			p->flash_count = p->type_flashing = 0;
		}
		p->pop_count -= timer_tick;
		return 1;
	}

	for (x = 0; x < PANEL_W; x++) {
		for (b =  p->blocks[x]; b && b->next; b = b->next)
			;
		for (; b; b = b->prev) {
			tmp = b->y;
			if (!b->next)
				b->y -= fmul(p->rise_speed, itofix(timer_tick));
		 	else if (b->glued)
				b->y = b->next->y - itofix(BLOCK_SIZE);
			else {
				b->y += fmul(p->fall_speed, itofix(timer_tick));
				if (b->next && (b->y + itofix(BLOCK_SIZE)) > b->next->y) {
					b->glued = 1;
					b->y = b->next->y - itofix(BLOCK_SIZE);
					colliding_blocks++;
				}
			}
			if (tmp != b->y || (b->next && b->y+itofix(BLOCK_SIZE) < b->next->y))
				ret = 1;
			if ((b->y >> 16) < 0)
				return -1;
			else if (!b->next && (b->y >> 16) <= (PANEL_H-1) * BLOCK_SIZE) {
				b->glued = 1;
				gen_bottom_row = 1;
			}
		}
	}

	if (gen_bottom_row)
		for (x = 0; x < PANEL_W; x++)
			block_add(p->blocks, x, itofix(PANEL_H*BLOCK_SIZE), RAND_BLOCK);

	if (colliding_blocks)
		play_sample(t->collide, 255, 128, 1000, 0);

	return ret;
}

/*
 * Check for sequences of blocks (3 or more), and mark the as popping
 * Returns the number of popping blocks found
 */
int check_blocks(Panel *p)
{
	int x, y, num_popping = 0;
	Block *b;
	struct {
		Block *block;
		int type;
		int pop;
	} blocks[PANEL_W][PANEL_H];

	if (p->blocks_popping)
		return 0;

	for (y = 0; y < PANEL_H; y++)
		for (x = 0; x < PANEL_W; x++) {
			blocks[x][y].block = NULL;
			blocks[x][y].type = blocks[x][y].pop = 0;
		}

	for (x = 0; x < PANEL_W; x++) {
		for (b = p->blocks[x]; b && b->next; b = b->next)
			;
		for (; b; b = b->prev) {
			if (b->next) {
				if (!b->glued)
					break;
				blocks[x][(b->y>>16)/BLOCK_SIZE].block = b;
				blocks[x][(b->y>>16)/BLOCK_SIZE].type = b->type;
			}
		}
	}

/* ugly macros for checking for a line of 3 of the same blocks */
#define COMP3V(a,b,c,x) (a >= 0 && b >= 0 && c >= 0 && a < PANEL_H && b < PANEL_H && c < PANEL_H && blocks[x][a].type == blocks[x][b].type && blocks[x][b].type == blocks[x][c].type)
#define COMP3H(a,b,c,y) (a >= 0 && b >= 0 && c >= 0 && a < PANEL_W && b < PANEL_W && c < PANEL_W && blocks[a][y].type == blocks[b][y].type && blocks[b][y].type == blocks[c][y].type)

	for (y = 0; y < PANEL_H; y++) 
		for (x = 0; x < PANEL_W; x++) {
			if (blocks[x][y].type && (
			COMP3V(y-2,y-1,y,x) || COMP3V(y,y+1,y+2,x) || COMP3V(y-1,y,y+1,x) ||
			COMP3H(x-2,x-1,x,y) || COMP3H(x,x+1,x+2,y) || COMP3H(x-1,x,x+1,y)))
			{
				blocks[x][y].block->popping = 1;
				num_popping++;
			}
		}

	if (num_popping) {
		p->blocks_popping = 1;
		p->pop_count = POP_DURATION;
		return num_popping;
	}

	return 0;
}

/*
 * Create a panel. x_pos,y_pos is it's top left position on screen.
 * min and max are the minimum and maximum number of blocks in each
 * column to start out with
 */
Panel *create_panel(int player_number, int x_pos, int y_pos, int min, int max)
{
	Panel *p;
	Block *b;
	int x, y, i, j;

	p = calloc(1, sizeof(Panel));
	if (!p)
		return NULL;

	p->player_number = player_number;

	p->bmp = create_sub_bitmap(vs,x_pos,y_pos,PANEL_W*BLOCK_SIZE,PANEL_H*BLOCK_SIZE);
	if (!p->bmp) {
		free(p);
		return NULL;
	}

	p->x = x_pos;
	p->y = y_pos;

	for (x = 0; x < PANEL_W; x++) {
		i = rand() % (max-min+1) + min;
		y = (PANEL_H - i) * BLOCK_SIZE;
		for (j = 0; j <= i; j++) {
			block_add(p->blocks, x, itofix(y), RAND_BLOCK);
			y += BLOCK_SIZE;
		}
	}
	
	for (x = 0; x < PANEL_W; x++) {
		for (b = p->blocks[x]; b; b = b->next)
			b->glued = 1;
	}

	p->fall_speed = ftofix(FALL_SPEED);
	p->rise_speed = ftofix(RISE_SPEED);

	return p;
}

void free_panel(Panel *p)
{
	Block *b, *next;
	int i;
	
	if (p->bmp)
		destroy_bitmap(p->bmp);
	for (i = 0; i < PANEL_W; i++) {
		b = p->blocks[i];
		while (b) {
			next = b->next;
			block_delete(p->blocks, b);
			b = next;
		}
	}

	if (p)
		free(p);
}

void draw_panel(Panel *p, Theme *t)
{
	BITMAP *block_bmp;
	Block *b;
	int x;

	for (x = 0; x < PANEL_W; x++)
		for (b = p->blocks[x]; b; b = b->next) {
			if (!b->popping || p->type_flashing == 0)
				block_bmp = t->blocks[b->type-1];
			else
				block_bmp = t->blocks_h[b->type-1];
			draw_sprite(p->bmp, block_bmp, b->x * BLOCK_SIZE, b->y >> 16);
		}
}

void draw_background(Panel *p1, Panel *p2, Theme *t, int timer_tick)
{
	BITMAP *b;

	b = t->background[t->background_frame];
	blit(b, vs, 0, 0, 0, 0, b->w, b->h);
	t->background_count -= timer_tick;
	if (t->background_count <= 0) {
		t->background_frame++;
		t->background_count = BACK_DURATION;
		if (!t->background[t->background_frame])
			t->background_frame = 0;
	}

	b = t->player_bitmaps[0][p1->player_bitmap];
	draw_sprite(vs, b, 0, 0);
	if (p1->player_bitmap != PLAYER_IDLE) {
		p1->player_count -= timer_tick;
		if (p1->player_count <= 0)
			p1->player_bitmap = PLAYER_IDLE;
	}

	b = t->player_bitmaps[1][p2->player_bitmap];
	draw_sprite(vs, b, vs->w - b->w, 0);
	if (p2->player_bitmap != PLAYER_IDLE) {
		p2->player_count -= timer_tick;
		if (p2->player_count <= 0)
			p2->player_bitmap = PLAYER_IDLE;
	}
}

void free_theme(Theme *t)
{
	if (t) {
		if (t->datafile)
			unload_datafile(t->datafile);
		free(t);
	}
}

Theme *load_theme(char *filename)
{
	Theme *theme;
	DATAFILE *d;
	int i;

	theme = malloc(sizeof(Theme));
	if (!theme)
		return NULL;

	d = theme->datafile = load_datafile(filename);
	if (!d) {
		free(theme);
		return NULL;
	}

	theme->preview = find_object_data(d, "PREVIEW");
	for (i = 0; i < MAX_BACKGROUND_FRAMES; i++)
		theme->background[i] = find_object_data(d, "BG%d", i);
	theme->blocks[0] = find_object_data(d, "BLOCK0");
	theme->blocks[1] = find_object_data(d, "BLOCK1");
	theme->blocks[2] = find_object_data(d, "BLOCK2");
	theme->blocks[3] = find_object_data(d, "BLOCK3");
	theme->blocks_h[0] = find_object_data(d, "BLOCK0_H");
	theme->blocks_h[1] = find_object_data(d, "BLOCK1_H");
	theme->blocks_h[2] = find_object_data(d, "BLOCK2_H");
	theme->blocks_h[3] = find_object_data(d, "BLOCK3_H");
	theme->player_bitmaps[0][PLAYER_IDLE] = find_object_data(d, "P1IDLE");
	theme->player_bitmaps[0][PLAYER_COMBO] = find_object_data(d, "P1COMBO");
	theme->player_bitmaps[0][PLAYER_WIN] = find_object_data(d, "P1WIN");
	theme->player_bitmaps[0][PLAYER_LOSE] = find_object_data(d, "P1LOSE");
	theme->player_samples[0][PLAYER_COMBO] = find_object_data(d, "P1COMBO_SAMP");
	theme->player_samples[0][PLAYER_WIN] = find_object_data(d, "P1WIN_SAMP");
	theme->player_samples[0][PLAYER_LOSE] = find_object_data(d, "P1LOSE_SAMP");
	theme->player_bitmaps[1][PLAYER_IDLE] = find_object_data(d, "P2IDLE");
	theme->player_bitmaps[1][PLAYER_COMBO] = find_object_data(d, "P2COMBO");
	theme->player_bitmaps[1][PLAYER_WIN] = find_object_data(d, "P2WIN");
	theme->player_bitmaps[1][PLAYER_LOSE] = find_object_data(d, "P2LOSE");
	theme->player_samples[1][PLAYER_COMBO] = find_object_data(d, "P2COMBO_SAMP");
	theme->player_samples[1][PLAYER_WIN] = find_object_data(d, "P2WIN_SAMP");
	theme->player_samples[1][PLAYER_LOSE] = find_object_data(d, "P2LOSE_SAMP");
	theme->start = find_object_data(d, "START");
	theme->collide = find_object_data(d, "COLLIDE");
	theme->pop = find_object_data(d, "POP");

	theme->background_frame = 0;
	theme->background_count = BACK_DURATION;

	return theme;
}
