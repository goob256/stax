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

void LoadData(void) throw (BadLoad)
{
	datafile = load_datafile("#");
	if (!datafile)
		throw BadLoad();

	char object_name[100];
	DATAFILE* object;

	for (int i = 0; i < NUMBER_OF_BLOCK_BITMAPS; i++) {
		sprintf(object_name, "BLOCK%d_BMP", i);
		object = find_datafile_object(datafile, object_name);
		if (object == NULL || object->dat == NULL)
			throw BadLoad();
		block_bitmaps[i] = (BITMAP*)object->dat;
		sprintf(object_name, "BLOCK%d_POPPING_BMP", i);
		object = find_datafile_object(datafile, object_name);
		if (object == NULL || object->dat == NULL)
			throw BadLoad();
		popping_block_bitmaps[i] = (BITMAP*)object->dat;
		for (int j = 0; j < FALL_FRAMES; j++) {
			sprintf(object_name, "FALLING_BLOCK%d-%d_BMP", i, j);
			object = find_datafile_object(datafile, object_name);
			if (object == NULL || object->dat == NULL)
				throw BadLoad();
			falling_block_bitmaps[i][j] = (BITMAP*)object->dat;
		}
		for (int j = 0; j < POP_FRAMES; j++) {
			sprintf(object_name, "pop%d_%d", i, j);
			object = find_datafile_object(datafile, object_name);
			if (object == NULL || object->dat == NULL)
				throw BadLoad();
			pop_bitmaps[i][j] = (BITMAP*)object->dat;
		}
	}

	object = find_datafile_object(datafile, "bg");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	background_bitmap = (BITMAP*)object->dat;

	object = find_datafile_object(datafile, "MAGNET_BMP");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	magnet_bitmap = (BITMAP*)object->dat;

	for (int i = 0; i < 4; i++) {
		char s[10];
		sprintf(s, "spring%d", i);
		object = find_datafile_object(datafile, s);
		if (object == NULL || object->dat == NULL)
			throw BadLoad();
		spring_bitmaps[i] = (BITMAP*)object->dat;
	}
	
	object = find_datafile_object(datafile, "SWAPPER_BMP");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	swapper_bitmap = (BITMAP*)object->dat;

	object = find_datafile_object(datafile, "arrow_r");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	arrow_r_bitmap = (BITMAP*)object->dat;

	object = find_datafile_object(datafile, "arrow_l");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	arrow_l_bitmap = (BITMAP*)object->dat;

	object = find_datafile_object(datafile, "pop");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	pop_sample = (SAMPLE*)object->dat;

	object = find_datafile_object(datafile, "suck");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	suck_sample = (SAMPLE*)object->dat;
	
	object = find_datafile_object(datafile, "thunk");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	thunk_sample = (SAMPLE*)object->dat;
	
	object = find_datafile_object(datafile, "toggle");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	toggle_sample = (SAMPLE*)object->dat;
	
	object = find_datafile_object(datafile, "charge");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	charge_sample = (SAMPLE*)object->dat;
	
	object = find_datafile_object(datafile, "blast");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	blast_sample = (SAMPLE*)object->dat;

	object = find_datafile_object(datafile, "nocharge");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	nocharge_sample = (SAMPLE*)object->dat;

	object = find_datafile_object(datafile, "intro");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	intro_midi = (MIDI*)object->dat;

	object = find_datafile_object(datafile, "game");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	game_midi = (MIDI*)object->dat;

	object = find_datafile_object(datafile, "font");
	if (object == NULL || object->dat == NULL)
		throw BadLoad();
	font = (FONT*)object->dat;
	
	DATAFILE *arrow_left_dat = NULL, *arrow_left_selected_dat = NULL, *arrow_left_depressed_dat = NULL;
	DATAFILE *arrow_right_dat = NULL, *arrow_right_selected_dat = NULL, *arrow_right_depressed_dat = NULL;
	DATAFILE *check_on_dat = NULL, *check_on_selected_dat = NULL, *check_on_depressed_dat = NULL;
	DATAFILE *check_off_dat = NULL, *check_off_selected_dat = NULL, *check_off_depressed_dat = NULL;
	DATAFILE *radio_on_dat = NULL, *radio_on_selected_dat = NULL, *radio_on_depressed_dat = NULL;
	DATAFILE *radio_off_dat = NULL, *radio_off_selected_dat = NULL, *radio_off_depressed_dat = NULL;
	DATAFILE *slider_tab_dat = NULL, *slider_tab_selected_dat = NULL, *slider_tab_depressed_dat = NULL;

	arrow_left_dat = find_datafile_object(datafile, "arrow_left");
	arrow_left_selected_dat = find_datafile_object(datafile, "arrow_left_selected");
	arrow_left_depressed_dat = find_datafile_object(datafile, "arrow_left_depressed");
	arrow_right_dat = find_datafile_object(datafile, "arrow_right");
	arrow_right_selected_dat = find_datafile_object(datafile, "arrow_right_selected");
	arrow_right_depressed_dat = find_datafile_object(datafile, "arrow_right_depressed");
	check_on_dat = find_datafile_object(datafile, "check_on");
	check_on_selected_dat = find_datafile_object(datafile, "check_on_selected");
	check_on_depressed_dat = find_datafile_object(datafile, "check_on_depressed");
	check_off_dat = find_datafile_object(datafile, "check_off");
	check_off_selected_dat = find_datafile_object(datafile, "check_off_selected");
	check_off_depressed_dat = find_datafile_object(datafile, "check_off_depressed");
	radio_on_dat = find_datafile_object(datafile, "radio_on");
	radio_on_selected_dat = find_datafile_object(datafile, "radio_on_selected");
	radio_on_depressed_dat = find_datafile_object(datafile, "radio_on_depressed");
	radio_off_dat = find_datafile_object(datafile, "radio_off");
	radio_off_selected_dat = find_datafile_object(datafile, "radio_off_selected");
	radio_off_depressed_dat = find_datafile_object(datafile, "radio_off_depressed");
	slider_tab_dat = find_datafile_object(datafile, "slider_tab");
	slider_tab_selected_dat = find_datafile_object(datafile, "slider_tab_selected");
	slider_tab_depressed_dat = find_datafile_object(datafile, "slider_tab_depressed");

	if (!arrow_left_dat || !arrow_left_selected_dat || !arrow_left_depressed_dat || 
			!arrow_right_dat || !arrow_right_selected_dat || !arrow_right_depressed_dat ||
		       	!check_on_dat || !check_on_selected_dat || !check_on_depressed_dat ||
			!check_off_dat || !check_off_selected_dat || !check_off_depressed_dat ||
		       	!radio_on_dat || !radio_on_selected_dat || !radio_on_depressed_dat ||
			!radio_off_dat || !radio_off_selected_dat || !radio_off_depressed_dat ||
			!slider_tab_dat || !slider_tab_selected_dat || !slider_tab_depressed_dat) {
		throw BadLoad();
	}
	if (!arrow_left_dat->dat || !arrow_left_selected_dat->dat || !arrow_left_depressed_dat->dat || 
			!arrow_right_dat->dat || !arrow_right_selected_dat->dat || !arrow_right_depressed_dat->dat ||
		       	!check_on_dat->dat || !check_on_selected_dat->dat || !check_on_depressed_dat->dat ||
			!check_off_dat->dat || !check_off_selected_dat->dat || !check_off_depressed_dat->dat ||
		       	!radio_on_dat->dat || !radio_on_selected_dat->dat || !radio_on_depressed_dat->dat ||
			!radio_off_dat->dat || !radio_off_selected_dat->dat || !radio_off_depressed_dat->dat ||
			!slider_tab_dat->dat || !slider_tab_selected_dat->dat || !slider_tab_depressed_dat->dat) {
		throw BadLoad();
	}
	
	arrow_left = (BITMAP*)arrow_left_dat->dat;
	arrow_left_selected = (BITMAP*)arrow_left_selected_dat->dat;
	arrow_left_depressed = (BITMAP*)arrow_left_depressed_dat->dat;
	arrow_right = (BITMAP*)arrow_right_dat->dat;
	arrow_right_selected = (BITMAP*)arrow_right_selected_dat->dat;
	arrow_right_depressed = (BITMAP*)arrow_right_depressed_dat->dat;
	check_on = (BITMAP*)check_on_dat->dat;
	check_on_selected = (BITMAP*)check_on_selected_dat->dat;
	check_on_depressed = (BITMAP*)check_on_depressed_dat->dat;
	check_off = (BITMAP*)check_off_dat->dat;
	check_off_selected = (BITMAP*)check_off_selected_dat->dat;
	check_off_depressed = (BITMAP*)check_off_depressed_dat->dat;
	radio_on = (BITMAP*)radio_on_dat->dat;
	radio_on_selected = (BITMAP*)radio_on_selected_dat->dat;
	radio_on_depressed = (BITMAP*)radio_on_depressed_dat->dat;
	radio_off = (BITMAP*)radio_off_dat->dat;
	radio_off_selected = (BITMAP*)radio_off_selected_dat->dat;
	radio_off_depressed = (BITMAP*)radio_off_depressed_dat->dat;
	slider_tab = (BITMAP*)slider_tab_dat->dat;
	slider_tab_selected = (BITMAP*)slider_tab_selected_dat->dat;
	slider_tab_depressed = (BITMAP*)slider_tab_depressed_dat->dat;

	ChangeColor(arrow_left, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(arrow_left_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(arrow_left_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(arrow_right, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(arrow_right_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(arrow_right_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(check_on, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(check_on_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(check_on_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(check_off, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(check_off_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(check_off_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(radio_on, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(radio_on_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(radio_on_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(radio_off, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(radio_off_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(radio_off_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(slider_tab, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b));
	ChangeColor(slider_tab_selected, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
	ChangeColor(slider_tab_depressed, makecol(255, 255, 255), makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b));
}

void DestroyData(void)
{
	unload_datafile(datafile);
}
