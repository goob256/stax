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

void InstallSound(void) throw (BadInstall)
{
	if (install_sound(configuration.sound_driver, configuration.midi_driver, NULL) == -1)
		throw BadInstall();
	set_volume(configuration.sound_volume, configuration.music_volume);
}

void RemoveSound(void)
{
	remove_sound();
}

void StopMusic(void)
{
	stop_midi();
}

void PlayIntroMusic(void)
{
	play_midi(intro_midi, 1);
}

void PlayGameMusic(void)
{
	play_midi(game_midi, 1);
}
