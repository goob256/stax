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

#include <iostream>
#include "stax.h"

void ProcessCommandLine(int argc, char** argv)
{
	high_score_filename = GetHighScoresFilename(argv[0]);
	ReadHighScores(high_score_filename);
	ReadConfiguration(argv[0]);

	for (int i = 1; i < argc; i++) {
		if (!stricmp(argv[i], "-windowed"))
			configuration.graphics_driver = GFX_AUTODETECT_WINDOWED;
		else if (!stricmp(argv[i], "-fullscreen"))
			configuration.graphics_driver = GFX_AUTODETECT_FULLSCREEN;
		else if (!stricmp(argv[i], "-sw")) {
			if (i+1 < argc) {
				i++;
				if (!isdigit(argv[i][0])) {
					std::cout << "Expected a number after -w." << std::endl;
					allegro_exit();
					exit(1);
				}
				configuration.screen_width = atoi(argv[i]);
			}
		}
		else if (!stricmp(argv[i], "-sh")) {
			if (i+1 < argc) {
				i++;
				if (!isdigit(argv[i][0])) {
					std::cout << "Expected a number after -h." << std::endl;
					allegro_exit();
					exit(1);
				}
				configuration.screen_height = atoi(argv[i]);
			}
		}
		else if (!stricmp(argv[i], "-bpp")) {
			if (i+1 < argc) {
				i++;
				if (!isdigit(argv[i][0])) {
					std::cout << "Expected a number after -bpp." << std::endl;
					allegro_exit();
					exit(1);
				}
				configuration.color_depth = atoi(argv[i]);
				if (!IsSupportedColorDepth(configuration.color_depth)) {
					std::cout << "Unsupported color depth." << std::endl;
					allegro_exit();
					exit(1);
				}
			}
		}
		else if (!stricmp(argv[i], "-blocks")) {
			if (i+1 < argc) {
				i++;
				if (!isdigit(argv[i][0])) {
					std::cout << "Expected a number after -blocks." << std::endl;
					allegro_exit();
					exit(1);
				}
				int blocks = atoi(argv[i]);
				if (blocks < 2 || blocks > NUMBER_OF_BLOCK_BITMAPS) {
					std::cout << "Number of block types must be from 2 to 5." << std::endl;
					allegro_exit();
					exit(1);
				}
				configuration.number_of_block_types = blocks;
			}
		}
		else if (!stricmp(argv[i], "-height")) {
			if (i+1 < argc) {
				i++;
				if (!isdigit(argv[i][0])) {
					std::cout << "Expected a number after -height." << std::endl;
					allegro_exit();
					exit(1);
				}
				configuration.initial_height = atoi(argv[i]);
				if (configuration.initial_height < 0) {
					std::cout << "Initial height must be 0 or greater." << std::endl;
					allegro_exit();
					exit(1);
				}
				else if (configuration.initial_height >= PANEL_HEIGHT) {
					std::cout << "Maximum initial height is " << PANEL_HEIGHT-1 << "." << std::endl;
					allegro_exit();
					exit(1);
				}
			}
		}
		else {
			std::cout << "Unrecognized command line option: " <<
				argv[i] << std::endl;
			std::cout << "Supported options are:" << std::endl;
			std::cout << "-windowed   -- Run in windowed mode" << std::endl;
			std::cout << "-fullscreen -- Run in fullscreen mode" << std::endl;
			std::cout << "-sw ###     -- Screen width" << std::endl;
			std::cout << "-sh ###     -- Screen height" << std::endl;
			std::cout << "-bpp ##     -- Color depth" << std::endl;
			std::cout << "-blocks #   -- Number of block types" << std::endl;
			std::cout << "-height ##  -- Initial height of blocks" << std::endl;
			allegro_exit();
			exit(1);
		}
	}
}

