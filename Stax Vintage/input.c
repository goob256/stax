#include <allegro.h>
#include "stax.h"

static void get_input_kb1(Input *input, int input_type, int timer_tick)
{
	poll_keyboard();

	input->left = key[KEY_A];
	input->right = key[KEY_D];
	input->up = key[KEY_W];
	input->down = key[KEY_S];
	input->button = key[KEY_SPACE];
}

static void get_input_kb2(Input *input, int input_type, int timer_tick)
{
	poll_keyboard();

	input->left = key[KEY_LEFT];
	input->right = key[KEY_RIGHT];
	input->up = key[KEY_UP];
	input->down = key[KEY_DOWN];
	input->button = key[KEY_RCONTROL];
}

static void get_input_joy1(Input *input, int input_type, int timer_tick)
{
	poll_joystick();

	input->left = joy[0].stick[0].axis[0].d1;
	input->right = joy[0].stick[0].axis[0].d2;
	input->up = joy[0].stick[0].axis[1].d1;
	input->down = joy[0].stick[0].axis[1].d2;
	input->button = joy[0].button[0].b;
}

static void get_input_joy2(Input *input, int input_type, int timer_tick)
{
	poll_joystick();

	input->left = joy[1].stick[0].axis[0].d1;
	input->right = joy[1].stick[0].axis[0].d2;
	input->up = joy[1].stick[0].axis[1].d1;
	input->down = joy[1].stick[0].axis[1].d2;
	input->button = joy[1].button[0].b;
}

void get_input(Input *input, int input_type, int timer_tick)
{
	switch (input_type) {
	case INPUT_KB1:
		get_input_kb1(input, input_type, timer_tick);
		break;
	case INPUT_KB2:
		get_input_kb2(input, input_type, timer_tick);
		break;
	case INPUT_JOY1:
		get_input_joy1(input, input_type, timer_tick);
		break;
	case INPUT_JOY2:
		get_input_joy2(input, input_type, timer_tick);
		break;
	}
}
