#include <stdio.h>
#include <time.h>
#include <allegro.h>

#include "stax.h"
#include "palette.h"

int tick = 0;

static void timer_callback(void)
{
	++tick;
}
END_OF_STATIC_FUNCTION(timer_callback);

BITMAP *vs = NULL;       /* virtual screen for double buffering */
DATAFILE *dat = NULL;
MIDI *intro_midi;
char *argv0;

Config cfg = 
{
	GAME_STYLE_SUCKA,    /* game style */
	"space.dat",         /* theme */
	INPUT_KB1,           /* player 1 input */
	INPUT_KB2,           /* player 2 input */
	1                    /* sound */
};

static void (*game_loops[NUM_GAME_STYLES])(void) =
{
	sucka_loop,
	clobber_loop,
	shifty_loop,
	coop_loop,
	puzzle_loop
};

static int init(void);
static void cleanup(void);

#define INTRO_START    0
#define INTRO_CONFIG   1
#define INTRO_EXIT     2
static int intro_screen(void);

int main(int argc, char **argv)
{
	int quit = 0;

	argv0 = argv[0];
   chdir("/usr/share/games/stax/");
	read_config();

	if (!init()) {
		cleanup();
		allegro_message("Error during initialization\n");
		return 1;
	}

	while (!quit) {
		switch (intro_screen()) {
			case INTRO_START:
				stop_midi();
				(*game_loops[cfg.style])();
				play_midi(intro_midi, 1);
				break;
			case INTRO_CONFIG:
				config_loop();
				break;
			case INTRO_EXIT:
				quit = 1;
				break;
		}
	}

	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

	return 0;
}
END_OF_MAIN();

static int init(void)
{
	srand(time(NULL));

	allegro_init();
	install_timer();
	install_keyboard();
	install_joystick(JOY_TYPE_AUTODETECT);
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);

	if (cfg.sound)
		set_volume(255, MIDI_VOLUME);
	else
		set_volume(0, 0);

	if ((dat = load_datafile("stax.dat")) == NULL) {
		allegro_message("Error loading stax.dat\n");
		return 0;
	}

	if ((vs = create_bitmap(320, 200)) == NULL) {
		allegro_message("Error creating virtual screen\n");
		return 0;
	}

	if (set_gfx_mode(GFX_SAFE, 320, 200, 0, 0) < 0) {
		allegro_message("Error: %s\n", allegro_error);
		return 0;
	}
	set_palette(palette);

	LOCK_VARIABLE(tick);
	LOCK_FUNCTION(timer_callback);
	install_int_ex(timer_callback, BPS_TO_TIMER(TIMER_FREQ));

	font = find_object_data(dat, "FONT");
	intro_midi = find_object_data(dat, "INTRO_MUSIC");

	return 1;
}

static void cleanup(void)
{
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

	if (vs) destroy_bitmap(vs);
	if (dat) unload_datafile(dat);
}

static int my_icon_proc(int, DIALOG *, int);

DIALOG intro_dialog[] =
{
	{ my_icon_proc,130,105,60,15,0,0,0,D_EXIT,0,0,NULL,NULL,NULL },
	{ my_icon_proc,130,120,60,15,0,0,0,D_EXIT,0,0,NULL,NULL,NULL },
	{ my_icon_proc,130,135,60,15,0,0,0,D_EXIT,0,0,NULL,NULL,NULL },
	{         NULL,  0,  0, 0, 0,0,0,0,     0,0,0,NULL,NULL,NULL }
};

static int my_icon_proc(int msg, DIALOG *d, int c)
{
	switch (msg) {
	case MSG_WANTFOCUS:
		return D_WANTFOCUS;
	case MSG_LOSTFOCUS:
		if (d->dp3)
			play_sample(d->dp3, 255, 128, 1000, 0);
		break;
	case MSG_DRAW:
		if (d->flags & D_GOTFOCUS) {
			if (d->dp2)
				stretch_sprite(screen, d->dp2, d->x, d->y, d->w, d->h);
		}
		else
			stretch_sprite(screen, d->dp, d->x, d->y, d->w, d->h);
		break;
	case MSG_KEY:
		play_sample(d->dp3, 255, 128, 800, 0);
		if (d->flags & D_EXIT)
			return D_CLOSE;
		break;
	}

	return D_O_K;
}

static int intro_screen(void)
{
	BITMAP *bg;
	BITMAP *logo;
	int ret = INTRO_EXIT;
	static int first_call = 1;

	bg = find_object_data(dat, "INTRO_BG");
	logo = find_object_data(dat, "STAX_LOGO");

	intro_dialog[0].dp = find_object_data(dat, "START");
	intro_dialog[0].dp2 = find_object_data(dat, "START_H");
	intro_dialog[1].dp = find_object_data(dat, "CONFIG");
	intro_dialog[1].dp2 = find_object_data(dat, "CONFIG_H");
	intro_dialog[2].dp = find_object_data(dat, "EXIT");
	intro_dialog[2].dp2 = find_object_data(dat, "EXIT_H");
	intro_dialog[0].dp3 = find_object_data(dat, "BUTTON");
	intro_dialog[1].dp3 = intro_dialog[2].dp3 = intro_dialog[3].dp3 = intro_dialog[0].dp3;

	/*
	 * Rise the background and spin in a logo.
	 * This enhances the "coolness" of the game, or something like that.
	 */
	if (first_call) {
		SAMPLE *spin = find_object_data(dat, "SPIN");
		BITMAP *nooss = find_object_data(dat, "NOOSS");
		BITMAP *tmp;
		fixed y = itofix(SCREEN_H/2);
		fixed angle = 0;
		fixed scale;
		int w, h;
		set_palette(black_palette);
		draw_sprite(screen, nooss, (SCREEN_W-nooss->w)/2, (SCREEN_H-nooss->h)/2);
		fade_in(palette, 1);
		fade_out(1);
		clear(screen);
		set_palette(palette);
		tmp = create_bitmap(logo->w, logo->h);
		if (tmp) {
			play_sample(spin, 255, 128, 1000, 0);
			for (scale = 0; scale <= itofix(1); scale += fmul(ftofix(0.0005), itofix(w))) {
				tick = 0;
				clear(tmp);
				w = logo->w * fixtof(scale);
				h = logo->h * fixtof(scale);
				stretch_sprite(tmp, logo, 0, 0, w, h);
				clear(vs);
				blit(bg, vs, 0, 0, 0, y>>16, SCREEN_W, SCREEN_W-(y>>16));
				pivot_sprite(vs, tmp, SCREEN_W/2, logo->h/2+5, w/2, h/2, angle);
				blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
				w = tick;
				y -= fmul(ftofix(0.05), itofix(w));
				angle += fmul(ftofix(0.256), itofix(w));
			}
		}
		blit(bg, vs, 0, 0, 0, 0, bg->w, bg->h);
		draw_sprite(vs, logo, (SCREEN_W-logo->w)/2, 5);
		blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
		play_midi(intro_midi, 1);
		first_call = 0;
	}
	else {
		blit(bg, vs, 0, 0, 0, 0, bg->w, bg->h);
		draw_sprite(vs, logo, (SCREEN_W-logo->w)/2, 5);
		blit(vs, screen, 0, 0, 0, 0, vs->w, vs->h);
	}

	ret = do_dialog(intro_dialog, 0);

	return ret;
}

