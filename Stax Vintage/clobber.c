/*
 * Code specific to the game "Clobber".
 */

#include <string.h>
#include <allegro.h>

#include "stax.h"

#define MOVE_SPEED             0.1
#define CHARGE_SPEED           0.0025
#define SPEED_INC              0.00005
#define MAX_CHARGE             4
#define MAX_CHARGES            99
#define START_CHARGES          5
#define FREE_CHARGE_DURATION   (TIMER_FREQ*10) /* every ten seconds */
#define NOCHARGE_DURATION      (TIMER_FREQ/4)

BITMAP *blaster;
SAMPLE *blast;
SAMPLE *crack;
SAMPLE *charge;
SAMPLE *nocharge;

/* game-specific info goes here */
typedef struct {
	fixed x;
	fixed charge;
	int charges;
	int block;
	int next_block;
	int free_charge_count;        /* inc charges when this reaches zero */
	int no_charge_count;
} GS;

static void draw_game_specific(Panel *, GS *, Theme *);
static void handle_input(Panel *, Input *, GS *, Theme *, int);

void clobber_loop(void)
{
	Panel *p1, *p2, *p;
	Block *b;
	Input i1, i2;
	GS gs1, gs2;
	Theme *t;
	int n, timer_tick = 0, quit = 0, new_game = 0;
	int i;

	t = load_theme(cfg.theme);
	if (!t) {
		error("Couldn't load the selected theme", "Sorry pal!", NULL);
		return;
	}

	blaster = find_object_data(dat, "BLASTER");
	crack = find_object_data(dat, "CRACK");
	blast = find_object_data(dat, "BLAST");
	charge = find_object_data(dat, "CHARGE");
	nocharge = find_object_data(dat, "NOCHARGE");

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

	p1->fall_speed = p2->fall_speed = ftofix(0.25);

	memset(&i1, 0, sizeof(Input));
	memset(&i2, 0, sizeof(Input));

	gs1.x = gs2.x = itofix(PANEL_W * BLOCK_SIZE / 2);
	gs1.charge = gs2.charge = 0;
	gs1.charges = gs2.charges = START_CHARGES;
	gs1.block = RAND_BLOCK;
	gs2.block = RAND_BLOCK;
	gs1.next_block = RAND_BLOCK;
	gs2.next_block = RAND_BLOCK;
	gs1.free_charge_count = gs2.free_charge_count = FREE_CHARGE_DURATION;
	gs1.no_charge_count = gs2.no_charge_count = 0;

	while (!quit) {
		tick = 0;
		draw_background(p1, p2, t, timer_tick);
		draw_panel(p1, t);
		draw_panel(p2, t);
		draw_game_specific(p1, &gs1, t);
		draw_game_specific(p2, &gs2, t);
		blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
		if (key[KEY_ESC]) {
			quit = !prompt("Really quit?", "Yes", "No");
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
		/* break blocks */
		for (n = 0; n < 2; n++) {
			if (n == 0)
				p = p1;
			else
				p = p2;
			for (i = 0; i < PANEL_W; i++) {
				for (b = p->blocks[i]; b; b = b->next) {
					if (b->data && b->next) {
						b->glued = 0;
						if (b->y >= b->next->y-itofix(BLOCK_SIZE)) {
							if (b->next->next) {
								b->data--;
								block_delete(p->blocks, b->next);
								play_sample(crack, 255, 128, 1000, 0);
							}
							else
								b->data = 0;
						}
					}
				}
			}
		}
		if ((gs1.free_charge_count -= timer_tick) <= 0 && gs1.charges < MAX_CHARGES) {
			gs1.charges++;
			gs1.free_charge_count = FREE_CHARGE_DURATION;
		}
		if ((gs2.free_charge_count -= timer_tick) <= 0 && gs2.charges < MAX_CHARGES) {
			gs2.charges++;
			gs2.free_charge_count = FREE_CHARGE_DURATION;
		}
		if ((n = check_blocks(p1)) > 3) {
			if (p1->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[0][PLAYER_COMBO], 255, 128, 1000, 0);
				p1->player_bitmap = PLAYER_COMBO;
				p1->player_count = PLAYER_DURATION;
			}
			gs1.charges += n-3;
			p2->rise_speed += fmul(itofix(n-3), ftofix(SPEED_INC));
		}
		if ((n = check_blocks(p2)) > 3) {
			if (p2->player_bitmap != PLAYER_COMBO) {
				play_sample(t->player_samples[1][PLAYER_COMBO], 255, 128, 1000, 0);
				p2->player_bitmap = PLAYER_COMBO;
				p2->player_count = PLAYER_DURATION;
			}
			gs2.charges += n-3;
			p1->rise_speed += fmul(itofix(n-3), ftofix(SPEED_INC));
		}
		if (gs1.charges > MAX_CHARGES) gs1.charges = MAX_CHARGES;
		if (gs2.charges > MAX_CHARGES) gs2.charges = MAX_CHARGES;
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
	int i;
	int size = 40 / MAX_CHARGE;

	/* show how many charges remaining */
	text_mode(-1);
	textprintf_shadow(vs, font, p->x+30, 11, C_WHITE, C_BLACK, "%2d", gs->charges);

	/* draw Charge-o-Meter(tm) */
	rect(vs, p->x+44, 10, p->x+86, 19, C_BLACK);
	for (i = 0; i < gs->charge>>16; i++)
		rectfill(vs, p->x+45+(size*i), 11, p->x+45+size+(size*i), 18, (C_GREEN+MAX_CHARGE*2)-(i*2));
	
	/* draw next block */
	draw_sprite(vs, t->blocks[gs->next_block-1], p->x+90, 10);

	/* draw blaster */
	draw_sprite(vs, t->blocks[gs->block-1],
		p->x + ((gs->x >> 16) / BLOCK_SIZE * BLOCK_SIZE), p->y - BLOCK_SIZE);
	draw_sprite(vs, blaster,
		p->x + ((gs->x >> 16) / BLOCK_SIZE * BLOCK_SIZE) - ((blaster->w - BLOCK_SIZE) / 2),
		p->y - blaster->h);
}

static void handle_input(Panel *p, Input *i, GS *gs, Theme *t, int timer_tick)
{
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

	if (gs->no_charge_count) {
		gs->no_charge_count -= timer_tick;
		if (gs->no_charge_count < 0)
			gs->no_charge_count = 0;
	}

	if (i->button) {
		fixed tmp = gs->charge;
		if (!gs->charges && !gs->no_charge_count) {
			play_sample(nocharge, 255, 128, 1000, 0);
			gs->no_charge_count = NOCHARGE_DURATION;
		}
		gs->charge += fmul(ftofix(CHARGE_SPEED), itofix(timer_tick));
		if (gs->charge>>16 > gs->charges)
			gs->charge = itofix(gs->charges);
		else if (gs->charge > itofix(MAX_CHARGE))
			gs->charge = itofix(MAX_CHARGE);
		if ((tmp>>16) != (gs->charge>>16))
			play_sample(charge, 255, 128, 1000+(50*(gs->charge>>16)), 0);
	}
	else {
		if (gs->charges && gs->charge && gs->charges >= gs->charge>>16
				&& p->blocks[x]->y >= itofix(BLOCK_SIZE))
		{
			play_sample(blast, 128+(32*(gs->charge>>16)), 128, 1000+(50*(gs->charge>>16)), 0);
			block_add(p->blocks, x, 0, gs->block);
			p->blocks[x]->data = gs->charge>>16;
			if (gs->charge>>16)
				gs->charges -= gs->charge>>16;
			else
				gs->charges--;
			gs->charge = 0;
			gs->block = gs->next_block;
			gs->next_block = RAND_BLOCK;
		}
		else if (gs->charge)
			gs->charge = 0;
	}
}
