#include <string.h>
#include <allegro.h>

#include "stax.h"

#define MOVE_SPEED            0.1
#define SUCK_SPEED            0.4
#define SPEED_DEC             0.00001
#define SPEED_INC             0.00005
#define SPEED_INC_DURATION    (TIMER_FREQ*5)

BITMAP *sucker;
BITMAP *blaster;
SAMPLE *suck;
SAMPLE *thunk;
SAMPLE *blast;

/* game-specific info goes here */
typedef struct {
	fixed x;
	int block;
} GS;

COLOR_MAP trans_table;
RGB_MAP rgb_table;

Panel *p1, *p2;
GS gs1, gs2;

static void draw_game_specific(Panel *, GS *, Theme *);
static void handle_input(Panel *, Input *, GS *, Theme *, int);

void coop_loop(void)
{
	Input i1, i2;
	Theme *t;
	int n, timer_tick = 0, quit = 0, new_game = 0;
	BITMAP *tmp_bmp;
	int speed_count = 0;
	int time = 0, min, sec;

	t = load_theme(cfg.theme);
	if (!t) {
		error("Couldn't load the selected theme", "Sorry pal!", NULL);
		return;
	}

	/* 
	 * since we don't use the WIN image or sound, use this trick so we can
	 * still use the game_over function
	 */

	t->player_bitmaps[0][PLAYER_WIN] = t->player_bitmaps[0][PLAYER_LOSE];
	t->player_bitmaps[1][PLAYER_WIN] = t->player_bitmaps[1][PLAYER_LOSE];
	t->player_samples[0][PLAYER_WIN] = t->player_samples[0][PLAYER_LOSE];
	t->player_samples[1][PLAYER_WIN] = t->player_samples[1][PLAYER_LOSE];

	create_rgb_table(&rgb_table, palette, NULL);
	rgb_map = &rgb_table;
	create_trans_table(&trans_table, palette, 100, 100, 100, NULL);
	color_map = &trans_table;

	sucker = find_object_data(dat, "MAGNET");
	blaster = find_object_data(dat, "BLASTER");
	suck = find_object_data(dat, "SUCK");
	thunk = find_object_data(dat, "THUNK");
	blast = find_object_data(dat, "BLAST");

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

	gs1.x = gs2.x = itofix(PANEL_W * BLOCK_SIZE / 2);
	gs1.block = gs2.block = 0;

	while (!quit) {
		tick = 0;
		time += timer_tick;
		speed_count += timer_tick;
		if (speed_count >= SPEED_INC_DURATION) {
			p1->rise_speed += ftofix(SPEED_INC);
			speed_count = 0;
		}
		draw_background(p1, p2, t, timer_tick);
		draw_panel(p1, t);
		tmp_bmp = p1->bmp;
		p1->bmp = p2->bmp;
		draw_panel(p1, t);
		p1->bmp = tmp_bmp;
		draw_game_specific(p1, &gs1, t);
		draw_game_specific(p2, &gs2, t);
		min = time / TIMER_FREQ / 60;
		sec = (time - (min * TIMER_FREQ * 60)) / TIMER_FREQ;
		text_mode(-1);
		textprintf_shadow(vs, font, SCREEN_W/2-15, 5, C_WHITE, C_BLACK,  "%2d:%02d", min, sec);
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
			stop_sample(suck);
			new_game = !game_over(p1, p2, t);
		}
		if ((n = check_blocks(p1)) > 3) {
			if (p1->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[0][PLAYER_COMBO], 255, 128, 1000, 0);
				p1->player_bitmap = PLAYER_COMBO;
				p1->player_count = PLAYER_DURATION;
			}
			if (p2->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[1][PLAYER_COMBO], 255, 128, 1000, 0);
				p2->player_bitmap = PLAYER_COMBO;
				p2->player_count = PLAYER_DURATION;
			}
			p1->rise_speed -= fmul(ftofix(SPEED_DEC), itofix(n-3));
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
	BITMAP *bmp1, *bmp2;
	int x1, x2;
	
	if (p->player_number == 0) {
		if (gs2.block)
			draw_trans_sprite(vs, t->blocks[gs2.block-1],
				p->x + ((gs2.x >> 16) / BLOCK_SIZE * BLOCK_SIZE), p->y - BLOCK_SIZE);
		bmp1 = sucker;
		bmp2 = blaster;
		x1 = p1->x + ((gs1.x >> 16) / BLOCK_SIZE * BLOCK_SIZE);
		x2 = p1->x + ((gs2.x >> 16) / BLOCK_SIZE * BLOCK_SIZE);
	}
	else {
		if (gs2.block)
			draw_sprite(vs, t->blocks[gs->block-1],
				p->x + ((gs2.x >> 16) / BLOCK_SIZE * BLOCK_SIZE), p->y - BLOCK_SIZE);
		bmp1 = blaster;
		bmp2 = sucker;
		x1 = p2->x + ((gs2.x >> 16) / BLOCK_SIZE * BLOCK_SIZE);
		x2 = p2->x + ((gs1.x >> 16) / BLOCK_SIZE * BLOCK_SIZE);
	}

	draw_trans_sprite(vs, bmp2, x2 - ((bmp2->w - BLOCK_SIZE) / 2),
		p->y - bmp2->h);

	draw_sprite(vs, bmp1, x1 - ((bmp1->w - BLOCK_SIZE) / 2),
		p->y - bmp1->h);
}

static void handle_input(Panel *p, Input *i, GS *gs, Theme *t, int timer_tick)
{
	static int sucking = 0, playing = 0;
	int x =  (gs->x >> 16) / BLOCK_SIZE;

	if (i->left && x) {
		gs->x -= fmul(ftofix(MOVE_SPEED), itofix(timer_tick));
		if ((gs->x >> 16) < 0)
			gs->x = 0;
	}
	else if (i->right && x < PANEL_W-1) {
		gs->x += fmul(ftofix(MOVE_SPEED), itofix(timer_tick));
		if ((gs->x >> 16) >= (PANEL_W-1) * BLOCK_SIZE)
			gs->x = itofix(PANEL_W-1) * BLOCK_SIZE;
	}

	x = (gs->x >> 16) / BLOCK_SIZE;

	if (p->player_number == 0) {
		if ((i->up || i->button) && !gs2.block
			&& (p->blocks[x]->y>>16) <= (PANEL_H-1) * BLOCK_SIZE
			&& (p->blocks[x]->popping ==  0))
		{
			sucking = 1;
			p->blocks[x]->glued = 0;
			if (p->blocks_popping)
				p->blocks[x]->y -= fmul(ftofix(SUCK_SPEED)-p->fall_speed, itofix(timer_tick));
			else
				p->blocks[x]->y -= fmul(ftofix(SUCK_SPEED), itofix(timer_tick));
			if (p->blocks[x]->y <= itofix(0)) {
				play_sample(thunk, 255, 128, 1000, 0);
				gs2.block = p->blocks[x]->type;
				block_delete(p->blocks, p->blocks[x]);
			}
		}
		else
			sucking = 0;

		if (sucking && !playing) {
			play_sample(suck, 255, 128, 1000, 1);
			playing = 1;
		}
		else if (!sucking) {
			stop_sample(suck);
			playing = 0;
		}
	}
	else if (i->button && gs->block && p1->blocks[x]->y >= itofix(BLOCK_SIZE)) {
		play_sample(blast, 255, 128, 1000, 0);
		block_add(p1->blocks, x, 0, gs->block);
		gs->block = 0;
	}
}
