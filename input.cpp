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

bool LeftPressed(void)
{
	return key[KEY_LEFT] || (configuration.joystick_installed ? joy[0].stick[0].axis[0].d1 : false);
}

bool RightPressed(void)
{
	return key[KEY_RIGHT] || (configuration.joystick_installed ? joy[0].stick[0].axis[0].d2 : false);
}

bool UpPressed(void)
{
	return key[KEY_UP] || (configuration.joystick_installed ? joy[0].stick[0].axis[1].d1 : false);
}

bool DownPressed(void)
{
	return key[KEY_DOWN] || (configuration.joystick_installed ? joy[0].stick[0].axis[1].d2 : false);
}

bool ButtonPressed(void)
{
	return key[KEY_ENTER] || (configuration.joystick_installed ? joy[0].button[configuration.joystick1_button].b : false);
}

void WaitForRelease(void)
{
	for (;;) {
		clear_keybuf();
		if (keyboard_needs_poll())
			poll_keyboard();
		if (mouse_needs_poll())
			poll_mouse();
		poll_joystick();
		if (	!key[KEY_ESC] &&
			!key[KEY_ENTER] &&
			(configuration.joystick_installed ?
			(!joy[0].button[0].b &&
			!joy[0].button[1].b &&
			!joy[0].button[2].b &
			!joy[0].button[3].b) : true) &&
			(configuration.mouse_installed ? !mouse_b : true))
			break;
	}
}

bool Input::LeftReady(void)
{
	return left_count <= 0;
}

bool Input::RightReady(void)
{
	return right_count <= 0;
}

bool Input::UpReady(void)
{
	return up_count <= 0;
}

bool Input::DownReady(void)
{
	return down_count <= 0;
}

bool Input::ButtonReady(void)
{
	return button_count <= 0;
}

void Input::PressLeft(void)
{
	left_repeat = MAX(MIN_REPEAT, left_repeat - MOVE_ACCEL);
	left_count = left_repeat;
}

void Input::PressRight(void)
{
	right_repeat = MAX(MIN_REPEAT, right_repeat - MOVE_ACCEL);
	right_count = right_repeat;
}

void Input::PressUp(void)
{
	if (init_up_repeat == 0)
		up_repeat = 0;
	else
		up_repeat = MAX(MIN_REPEAT, up_repeat - MOVE_ACCEL);
	up_count = up_repeat;
}

void Input::PressDown(void)
{
	if (init_down_repeat == 0)
		down_repeat = 0;
	else
		down_repeat = MAX(MIN_REPEAT, down_repeat - MOVE_ACCEL);
	down_count = down_repeat;
}

void Input::PressButton(void)
{
	button_count = button_repeat;
}

void Input::SetRepeatTimes(int left, int right, int up, int down, int button)
{
	left_repeat = left;
	right_repeat = right;
	up_repeat = up;
	down_repeat = down;
	button_repeat = button;
	init_left_repeat = left;
	init_right_repeat = right;
	init_up_repeat = up;
	init_down_repeat = down;
	init_button_repeat = button;
}

void Input::Update(void)
{
	if (left_count > 0)
		left_count--;
	if (right_count > 0)
		right_count--;
	if (up_count > 0)
		up_count--;
	if (down_count > 0)
		down_count--;
	if (button_count > 0)
		button_count--;
}

Input::Input(void)
{
	left_repeat = 0;
	left_count = 0;
	right_repeat = 0;
	right_count = 0;
	up_repeat = 0;
	up_count = 0;
	down_repeat = 0;
	down_count = 0;
	button_repeat = 0;
	button_count = 0;
}

bool LeftKeyboardInput::LeftPressed(void)
{
	if (LeftReady()) {
		if (key[KEY_A]) {
			PressLeft();
			return true;
		}
		else
			left_repeat = init_left_repeat;
	}
	return false;
}

bool LeftKeyboardInput::RightPressed(void)
{
	if (RightReady()) {
		if (key[KEY_D]) {
			PressRight();
			return true;
		}
		else
			right_repeat = init_right_repeat;
	}
	return false;
}

bool LeftKeyboardInput::UpPressed(void)
{
	if (UpReady()) {
		if (key[KEY_W]) {
			PressUp();
			return true;
		}
		else
			up_repeat = init_up_repeat;
	}
	return false;
}

bool LeftKeyboardInput::DownPressed(void)
{
	if (DownReady()) {
		if (key[KEY_S]) {
			PressDown();
			return true;
		}
		else
			down_repeat = init_down_repeat;
	}
	return false;
}

bool LeftKeyboardInput::ButtonPressed(void)
{
	if (ButtonReady()) {
		if (key[KEY_SPACE]) {
			PressButton();
			return true;
		}
		else
			button_repeat = init_button_repeat;
	}
	return false;
}

void LeftKeyboardInput::Update(void)
{
	if (keyboard_needs_poll())
		poll_keyboard();
	Input::Update();
}

bool RightKeyboardInput::LeftPressed(void)
{
	if (LeftReady()) {
		if (key[KEY_LEFT]) {
			PressLeft();
			return true;
		}
		else
			left_repeat = init_left_repeat;
	}
	return false;
}

bool RightKeyboardInput::RightPressed(void)
{
	if (RightReady()) {
		if (key[KEY_RIGHT]) {
			PressRight();
			return true;
		}
		else
			right_repeat = init_right_repeat;
	}
	return false;
}

bool RightKeyboardInput::UpPressed(void)
{
	if (UpReady()) {
		if (key[KEY_UP]) {
			PressUp();
			return true;
		}
		else
			up_repeat = init_up_repeat;
	}
	return false;
}

bool RightKeyboardInput::DownPressed(void)
{
	if (DownReady()) {
		if (key[KEY_DOWN]) {
			PressDown();
			return true;
		}
		else
			down_repeat = init_down_repeat;
	}
	return false;
}

bool RightKeyboardInput::ButtonPressed(void)
{
	if (ButtonReady()) {
		if (key[KEY_RCONTROL] || key[KEY_LCONTROL]) {
			PressButton();
			return true;
		}
		else
			button_repeat = init_button_repeat;
	}
	return false;
}

void RightKeyboardInput::Update(void)
{
	if (keyboard_needs_poll())
		poll_keyboard();
	Input::Update();
}

bool Joystick1Input::LeftPressed(void)
{
	if (LeftReady()) {
		if (joy[0].stick[0].axis[0].d1) {
			PressLeft();
			return true;
		}
		else
			left_repeat = init_left_repeat;
	}
	return false;
}

bool Joystick1Input::RightPressed(void)
{
	if (RightReady()) {
		if (joy[0].stick[0].axis[0].d2) {
			PressRight();
			return true;
		}
		else
			right_repeat = init_right_repeat;
	}
	return false;
}

bool Joystick1Input::UpPressed(void)
{
	if (UpReady()) {
		if (joy[0].stick[0].axis[1].d1) {
			PressUp();
			return true;
		}
		else
			up_repeat = init_up_repeat;
	}
	return false;
}

bool Joystick1Input::DownPressed(void)
{
	if (DownReady()) {
		if (joy[0].stick[0].axis[1].d2) {
			PressDown();
			return true;
		}
		else
			down_repeat = init_down_repeat;
	}
	return false;
}

bool Joystick1Input::ButtonPressed(void)
{
	if (ButtonReady()) {
		if (joy[0].button[configuration.joystick1_button].b) {
			PressButton();
			return true;
		}
		else
			button_repeat = init_button_repeat;
	}
	return false;
}

void Joystick1Input::Update(void)
{
	poll_joystick();
	Input::Update();
}

bool Joystick2Input::LeftPressed(void)
{
	if (LeftReady()) {
		if (joy[1].stick[0].axis[0].d1) {
			PressLeft();
			return true;
		}
		else
			left_repeat = init_left_repeat;
	}
	return false;
}

bool Joystick2Input::RightPressed(void)
{
	if (RightReady()) {
		if (joy[1].stick[0].axis[0].d2) {
			PressRight();
			return true;
		}
		else
			right_repeat = init_right_repeat;
	}
	return false;
}

bool Joystick2Input::UpPressed(void)
{
	if (UpReady()) {
		if (joy[1].stick[0].axis[1].d1) {
			PressUp();
			return true;
		}
		else
			up_repeat = init_up_repeat;
	}
	return false;
}

bool Joystick2Input::DownPressed(void)
{
	if (DownReady()) {
		if (joy[1].stick[0].axis[1].d2) {
			PressDown();
			return true;
		}
		else
			down_repeat = init_down_repeat;
	}
	return false;
}

bool Joystick2Input::ButtonPressed(void)
{
	if (ButtonReady()) {
		if (joy[1].button[configuration.joystick1_button].b) {
			PressButton();
			return true;
		}
		else
			button_repeat = init_button_repeat;
	}
	return false;
}

void Joystick2Input::Update(void)
{
	poll_joystick();
	Input::Update();
}

Input* CreateInput(int player)
{
	InputType input_type;

	if (player == 1)
		input_type = configuration.input_type_1;
	else if (player == 2)
		input_type = configuration.input_type_2;
	else
		return 0;

	switch (input_type) {
		case INPUT_NONE:
			return 0;
		case INPUT_RIGHT_KEYBOARD:
			return new RightKeyboardInput();
		case INPUT_LEFT_KEYBOARD:
			return new LeftKeyboardInput();
		case INPUT_JOYSTICK_1:
			return new Joystick1Input();
		case INPUT_JOYSTICK_2:
			return new Joystick2Input();
		default:
			return 0;
	}
}
