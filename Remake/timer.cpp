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

#include <allegro.h>

#ifdef UNIX
#include <sys/time.h>
#else
#include <winalleg.h>
#endif

#include "stax.h"

/*
static void IncrementTick(void)
{
	tick++;
}
END_OF_STATIC_FUNCTION(IncrementTick);

void InstallTimer(void) throw (BadInstall)
{
	LOCK_VARIABLE(tick);
	LOCK_FUNCTION(IncrementTick);

	if (install_timer() != 0)
		throw BadInstall();

	if (install_int_ex(IncrementTick, BPS_TO_TIMER(1000)) != 0)
		throw BadInstall();
}

void RemoveTimer(void)
{
	remove_int(IncrementTick);
	remove_timer();
}
*/

void UpdateFallingBlocks(int step)
{
	static int count = 0;
	count += step;
	if (count > ROTATE_DELAY) {
		count -= ROTATE_DELAY;
		falling_block_frame++;
		if (falling_block_frame >= FALL_FRAMES)
			falling_block_frame = 0;
	}
}

long currentTimeMillis()
{
#ifdef UNIX
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#else
	return timeGetTime();
#endif
}
