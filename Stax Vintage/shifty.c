/*
 * Code specific to the game "Shifty"
 */

#include <string.h>
#include <allegro.h>

#include "stax.h"

#define MOVE_DURATION   (TIMER_FREQ/8)
#define SHIFT_DURATION  (TIMER_FREQ*2)
#define SPEED_INC       0.00005

/* game-specific info goes here */
typedef struct {
	int y;
	int row;
	int move_count;
	int shift_count;
	fixed rise_count;
} GS;

static void draw_game_specific(Panel *, GS *, Theme *);
static void handle_input(Panel *, Input *, GS *, Theme *, int);

void shifty_loop(void)
{
	Panel *p1, *p2;
	Input i1, i2;
	GS gs1, gs2;
	Theme *t;
	int n, timer_tick = 0, quit = 0, new_game = 0;

	t = load_theme(cfg.theme);
	if (!t) {
		error("Couldn't load the selected theme", "Sorry pal!", NULL);
		return;
	}

newgame:
	play_sample(t->start, 255, 128, 1000, 0);
	zoom_image(t->background[0]);

	p1 = create_panel(0, 15, 35, 2, 4);
	p2 = create_panel(1, 165, 35, 2, 4);

	if (!p1 || !p2) {
		if (p1)
			free_panel(p1);
		error("Out of memory!", NULL, NULL);
		return;
	}

	p1->fall_speed = p2->fall_speed = ftofix(0.15);

	memset(&i1, 0, sizeof(Input));
	memset(&i2, 0, sizeof(Input));

	gs1.y = gs2.y = (PANEL_H-1) * BLOCK_SIZE;
	gs1.row = gs2.row = 1;
	gs1.rise_count = gs2.rise_count = 0;
	gs1.move_count = gs2.move_count = 0;
	gs1.shift_count = gs2.shift_count = 0;

	while (!quit) {
		tick = 0;
		draw_background(p1, p2, t, timer_tick);
		draw_panel(p1, t);
		draw_panel(p2, t);
		draw_game_specific(p1, &gs1, t);
		draw_game_specific(p2, &gs2, t);
		blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
		if (key[KEY_ESC]) {
			quit = !prompt("Really Quit?", "Yes", "No");
			continue;
		}
		else if (key[KEY_P]) {
			n = tick;
			do_pause();
			tick = n;
		}
		else if (key[KEY_BACKSPACE])
			screenshot();
		get_input(&i1, cfg.input1, timer_tick);
		get_input(&i2, cfg.input2, timer_tick);
		handle_input(p1, &i1, &gs1, t, timer_tick);
		handle_input(p2, &i2, &gs2, t, timer_tick);
		if (move_blocks(p1, t, timer_tick) < 0) {
			quit = 1;
			new_game = !game_over(p2, p1, t);
		}
		if (move_blocks(p2, t, timer_tick) < 0) {
			quit = 1;
			new_game = !game_over(p1, p2, t);
		}
		if ((n = check_blocks(p1)) > 3) {
			if (p1->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[0][PLAYER_COMBO], 255, 128, 1000, 0);
				p1->player_bitmap = PLAYER_COMBO;
				p1->player_count = PLAYER_DURATION;
			}
			p2->rise_speed += fmul(itofix(n-3), ftofix(SPEED_INC));
		}
		if ((n = check_blocks(p2)) > 3) {
			if (p2->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[1][PLAYER_COMBO], 255, 128, 1000, 0);
				p2->player_bitmap = PLAYER_COMBO;
				p2->player_count = PLAYER_DURATION;
			}
			p1->rise_speed += fmul(itofix(n-3), ftofix(SPEED_INC));
		}
		timer_tick = tick;
	}

	free_panel(p1);
	free_panel(p2);
	
	if (new_game) {
		new_game = quit = 0;
		goto newgame;
	}
	
	free_theme(t);
	clear_keybuf();
}

static void draw_game_specific(Panel *p, GS *gs, Theme *t)
{
	int x = p->x;
	int y = gs->y + 34;

	line(vs, x, y, x, y + BLOCK_SIZE, C_WHITE);
	line(vs, x+1, y+1, x+1, y + BLOCK_SIZE - 1, C_BLACK);
	line(vs, x, y, x + BLOCK_SIZE, y, C_WHITE);
	line(vs, x+1, y+1, x + BLOCK_SIZE, y+1, C_BLACK);
	line(vs, x, y + BLOCK_SIZE, x + BLOCK_SIZE, y + BLOCK_SIZE, C_WHITE);
	line(vs, x, y + BLOCK_SIZE+1, x + BLOCK_SIZE, y + BLOCK_SIZE+1, C_BLACK);

	x += PANEL_W * BLOCK_SIZE;
	
	line(vs, x, y, x, y + BLOCK_SIZE, C_WHITE);
	line(vs, x-1, y+1, x-1, y + BLOCK_SIZE - 1, C_BLACK);
	line(vs, x, y, x - BLOCK_SIZE, y, C_WHITE);
	line(vs, x-1, y+1, x - BLOCK_SIZE, y+1, C_BLACK);
	line(vs, x, y + BLOCK_SIZE, x - BLOCK_SIZE, y + BLOCK_SIZE, C_WHITE);
	line(vs, x, y + BLOCK_SIZE+1, x - BLOCK_SIZE, y + BLOCK_SIZE+1, C_BLACK);

	x = 50 - ((double)gs->shift_count/(double)SHIFT_DURATION * 50.0);
	rectfill(vs, p->x+48, 14, p->x+101, 18, C_WHITE);
	rectfill(vs, p->x+49, 15, p->x+100, 17, C_BLACK);
	for (y = 0; y < x; y++)
		putpixel(vs, p->x+50+y, 16, C_GREEN+10-(y/5));
}

static void handle_input(Panel *p, Input *i, GS *gs, Theme *t, int timer_tick)
{
	int x, y, tmp_type;
	fixed bottom = 0, tmp_y = 0;
	Block *b;
	Block *blocks[PANEL_W][PANEL_H];
	int movable[PANEL_H];

	if (gs->move_count) {
		gs->move_count -= timer_tick;
		if (gs->move_count < 0)
			gs->move_count = 0;
	}
	if (gs->shift_count) {
		gs->shift_count -= timer_tick;
		if (gs->shift_count < 0)
			gs->shift_count = 0;
	}

	for (y = 0; y < PANEL_H; y++) {
		for (x = 0; x < PANEL_W; x++)
			blocks[x][y] = NULL;
		movable[y] = 1;
	}

	for (x = 0; x < PANEL_W; x++) {
		for (b = p->blocks[x]; b && b->next; b = b->next)
			;
		if (!x) {
			bottom = b->y;
			if ((b->y - fmul(p->rise_speed, itofix(timer_tick))) >> 16 <= (PANEL_H-1) * BLOCK_SIZE) {
				gs->row++;
				if (gs->row == PANEL_H)
					gs->row--;
				else
					bottom += itofix(BLOCK_SIZE);
			}
		}
		for (; b; b = b->prev) {
			if (b->next) {
				if (!b->glued)
					break;
				blocks[x][(b->y>>16)/BLOCK_SIZE] = b;
			}
		}
		if (b) {
			for (b = b->prev; b; b = b->prev) {
				movable[(b->y>>16)/BLOCK_SIZE] = 0;
				if ((b->y>>16) % BLOCK_SIZE && (b->y>>16)/BLOCK_SIZE < PANEL_H)
					movable[(b->y>>16)/BLOCK_SIZE] = 0;
			}
		}
	}

	if (i->up && gs->row < PANEL_H-1 && !gs->move_count) {
		gs->row++;
		gs->move_count = MOVE_DURATION;
	}
	if (i->down && gs->row > 1 && !gs->move_count) {
		gs->row--;
		gs->move_count = MOVE_DURATION;
	}

	gs->y = (bottom - itofix(BLOCK_SIZE * gs->row)) >> 16;

	if (p->blocks_popping)
		return;

	y = PANEL_H-gs->row-1;

	if (i->left && !gs->shift_count) {
		if (movable[y]) {
			if (blocks[0][y]) {
				tmp_type = blocks[0][y]->type;
				tmp_y = blocks[0][y]->y;
			}
			else
				tmp_type = 0;
			for (x = 0; x < PANEL_W-1; x++) {
				if (blocks[x][y])
					block_delete(p->blocks, blocks[x][y]);
				if (blocks[x+1][y])
					block_add(p->blocks, x, blocks[x+1][y]->y, blocks[x+1][y]->type);
			}
			if (blocks[PANEL_W-1][y])
				block_delete(p->blocks, blocks[PANEL_W-1][y]);
			if (tmp_type)
				block_add(p->blocks, PANEL_W-1, tmp_y, tmp_type);
		}
		gs->shift_count = SHIFT_DURATION;
	}
	if (i->right && !gs->shift_count) {
		if (movable[y]) {
			if (blocks[PANEL_W-1][y]) {
				tmp_type = blocks[PANEL_W-1][y]->type;
				tmp_y = blocks[PANEL_W-1][y]->y;
			}
			else
				tmp_type = 0;
			for (x = PANEL_W-1; x > 0; x--) {
				if (blocks[x][y])
					block_delete(p->blocks, blocks[x][y]);
				if (blocks[x-1][y])
					block_add(p->blocks, x, blocks[x-1][y]->y, blocks[x-1][y]->type);
			}
			if (blocks[0][y])
				block_delete(p->blocks, blocks[0][y]);
			if (tmp_type)
				block_add(p->blocks, 0, tmp_y, tmp_type);
		}
		gs->shift_count = SHIFT_DURATION;
	}
}
