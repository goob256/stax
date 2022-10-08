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

#include <stdarg.h>
#include <allegro.h>

#include "stax.h"

void SetGraphicsMode(void) throw (BadGraphicsMode)
{
	set_color_depth(configuration.color_depth);
	if (set_gfx_mode(configuration.graphics_driver, configuration.screen_width, configuration.screen_height, 0, 0) < 0)
		throw BadGraphicsMode();
	configuration.graphics_mode_set = true;
}

void CreateBuffer(void) throw (std::bad_alloc)
{
	buffer = create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
	if (!buffer)
		throw std::bad_alloc();
}

void DestroyBuffer(void)
{
	destroy_bitmap(buffer);
}

void DrawBlock(int x, int y, Block* block, bool falling)
{
	if (block->type < 0)
		return;

	const int y_cutoff = PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE);

	int draw_height	= y_cutoff - y;
	if (draw_height > BLOCK_SIZE)
		draw_height = BLOCK_SIZE;

	if (draw_height <= 0)
		return;

	int color;

	BITMAP* bmp;

	if (falling)
		bmp = falling_block_bitmaps[block->type][falling_block_frame];
	else if (block->popping && (((unsigned)currentTimeMillis() % (POP_FLASH_TIME*2)) < POP_FLASH_TIME)) {
		bmp = popping_block_bitmaps[block->type];
	}
	else
		bmp = block_bitmaps[block->type];

	masked_blit(bmp, buffer, 0, 0, x, y, BLOCK_SIZE, draw_height);
}

void FillRectangle(int x1, int y1, int x2, int y2)
{
	int color = makecol(0, 0, 0);
	rectfill(buffer, x1, y1, x2, y2, color);
}

void DrawSunkenRectangle(int x1, int y1, int x2, int y2)
{
	int bright_color = makecol(100, 100, 200);
	int dark_color = makecol(50, 50, 50);

	line(buffer, x1, y1, x2, y1, dark_color);
	line(buffer, x1, y1+1, x1, y2, dark_color);
	line(buffer, x1+1, y2, x2, y2, bright_color);
	line(buffer, x2, y1+1, x2, y2-1, bright_color);
	
	line(buffer, x1-1, y1-1, x2+1, y1-1, dark_color);
	line(buffer, x1-1, y1, x1-1, y2+1, dark_color);
	line(buffer, x1, y2+1, x2+1, y2+1, bright_color);
	line(buffer, x2+1, y1, x2+1, y2, bright_color);
}

void BlitToScreen(void)
{
	if (in_gui) {
		float x_ratio = (float)BUFFER_WIDTH / (float)SCREEN_W;
		float y_ratio = (float)BUFFER_HEIGHT / (float)SCREEN_H;
		int mx = (int)((float)mouse_x * x_ratio);
		int my = (int)((float)mouse_y * y_ratio);
		draw_sprite(buffer, mouse_sprite, mx, my);
	}
	if (SCREEN_W == BUFFER_WIDTH && SCREEN_H == BUFFER_HEIGHT) {
		blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	}
	else {
		stretch_blit(buffer, screen, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, 0, 0, SCREEN_W, SCREEN_H);
	}
}

bool IsSupportedColorDepth(int color_depth)
{
	if (color_depth != 32 && color_depth != 24 && color_depth != 16 && color_depth != 15)
		return false;
	else
		return true;
}

void DrawText(int x, int y, bool center, char* format, ...)
{
   char text[1000];

   va_list ap;
   va_start(ap, format);
   uvsprintf(text, format, ap);
   va_end(ap);

   if (center) {
	int width = text_length(font, "x") * strlen(text);
	x = x - (width / 2);
   }

   textout_ex(buffer, font, text, x-1, y, makecol(0, 0, 0), -1);
   textout_ex(buffer, font, text, x+1, y, makecol(0, 0, 0), -1);
   textout_ex(buffer, font, text, x, y-1, makecol(0, 0, 0), -1);
   textout_ex(buffer, font, text, x, y+1, makecol(0, 0, 0), -1);
   int text_color = makecol(configuration.colors[COLOR_TEXT].r, configuration.colors[COLOR_TEXT].g, configuration.colors[COLOR_TEXT].b);
   textout_ex(buffer, font, text, x, y, text_color, -1);
}

void ChangeColor(BITMAP *b, int from, int to)
{
	for (int y = 0; y < b->h; y++) {
		for (int x = 0; x < b->w; x++) {
			int c = getpixel(b, x, y);
			if (c == from) {
				c = to;
			}
			putpixel(b, x, y, c);
		}
	}
}

/*
void DrawBackgroundBitmap(void)
{
	for (int y = 0; y < BUFFER_HEIGHT; y++) {
		float p = (float)y / (float)BUFFER_HEIGHT * 100.0;
		int c = GUI_GetGradientColor(100.0 - p, GUI_TOP_COLOR, GUI_BOTTOM_COLOR);
		line(background_bitmap, 0, y, BUFFER_WIDTH-1, y, c);
	}
}
*/

void SaveScreenshot(void)
{
	save_bmp("ss.bmp", buffer, NULL);
	rest(1000);
}
