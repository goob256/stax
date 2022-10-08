/*
 * Code specific to the game "Sucka".
 */

#include <string.h>
#include <allegro.h>

#include "stax.h"

#define MOVE_SPEED 0.1
#define SUCK_SPEED 0.4
#define SPEED_INC  0.00005

BITMAP *sucker;
SAMPLE *suck;
SAMPLE *thunk;

/* game-specific info goes here */
typedef struct {
	fixed x;
	int block;
} GS;

static void draw_game_specific(Panel *, GS *, Theme *);
static void handle_input(Panel *, Input *, GS *, Theme *, int);

void sucka_loop(void)
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

	sucker = find_object_data(dat, "MAGNET");
	suck = find_object_data(dat, "SUCK");
	thunk = find_object_data(dat, "THUNK");

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
			stop_sample(suck);
			new_game = !game_over(p2, p1, t);
		}
		if (move_blocks(p2, t, timer_tick) < 0) {
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
	if (gs->block)
		draw_sprite(vs, t->blocks[gs->block-1],
			p->x + ((gs->x >> 16) / BLOCK_SIZE * BLOCK_SIZE), p->y - BLOCK_SIZE);
	draw_sprite(vs, sucker,
		p->x + ((gs->x >> 16) / BLOCK_SIZE * BLOCK_SIZE) - ((sucker->w - BLOCK_SIZE) / 2),
		p->y - sucker->h);
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


	if ((i->up || i->button) && !gs->block 
		&& (p->blocks[x]->y>>16) <= (PANEL_H-1) * BLOCK_SIZE
		&& (p->blocks[x]->popping ==  0))
	{
		sucking |= p->player_number+1;
		p->blocks[x]->glued = 0;
		if (p->blocks_popping)
			p->blocks[x]->y -= fmul(ftofix(SUCK_SPEED)-p->fall_speed, itofix(timer_tick));
		else
			p->blocks[x]->y -= fmul(ftofix(SUCK_SPEED), itofix(timer_tick));
		if (p->blocks[x]->y <= itofix(0)) {
			play_sample(thunk, 255, 128, 1000, 0);
			gs->block = p->blocks[x]->type;
			block_delete(p->blocks, p->blocks[x]);
		}
	}
	else
		sucking &= ~(p->player_number+1);

	if (i->down && gs->block) {
		if (p->blocks[x]->y >= itofix(BLOCK_SIZE)) {
			block_add(p->blocks, x, 0, gs->block);
			gs->block = 0;
		}
	}

	if (sucking && !playing) {
		play_sample(suck, 255, 128, 1000, 1);
		playing = 1;
	}
	else if (!sucking) {
		stop_sample(suck);
		playing = 0;
	}
}
