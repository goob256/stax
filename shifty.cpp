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

bool Shifty(void)
{
	input1 = CreateInput(1);
	input1->SetRepeatTimes(150, 150, 150, 150, 150);
	input2 = CreateInput(2);
	if (input2)
		input2->SetRepeatTimes(150, 150, 150, 150, 150);

	if (input2 == 0) {
		panel1 = new Panel(SINGLE_PLAYER_PANEL_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
		panel1->SetRiseIncrement(SHIFTY_RISE_INCREMENT);
	}
	else {
		panel1 = new Panel(PANEL1_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
		panel1->SetRiseIncrement(SHIFTY_RISE_INCREMENT);
		panel2 = new Panel(PANEL2_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
		panel2->SetRiseIncrement(SHIFTY_RISE_INCREMENT);
	}

	int player1_row = PANEL_HEIGHT-1;
	int player2_row = PANEL_HEIGHT-1;
	int player1_col = PANEL_WIDTH/2;
	int player2_col = PANEL_WIDTH/2;
	int bottom_height = 0;
	int last_bottom_height = 0;

	int winner = -1;

	bool done = false;
	int last_tick = currentTimeMillis();;

	bool esc_pressed = false;

	for (;;) {
		long current_time = currentTimeMillis();
		int steps = MIN(100, current_time - last_tick);
		last_tick = current_time;
		for (int step = MIN(2, steps); steps > 0; steps -= 2, step = MIN(2, steps)) {
			UpdateFallingBlocks(step);
			if (key[KEY_BACKSPACE])
				SaveScreenshot();
			bottom_height = panel1->GetBottomHeight();
			if (last_bottom_height > bottom_height) {
				player1_row--;
				if (player1_row < 0)
					player1_row = 0;
				player2_row--;
				if (player2_row < 0)
					player2_row = 0;
			}
			last_bottom_height = bottom_height;
			if (panel1->MoveBlocks(step)) {
				// game over
				if (input2 && panel2->MoveBlocks(step))
					winner = 3;
				else
					winner = 2;
				done = true;
				break;
			}
			if (input2 != 0) {
				if (panel2->MoveBlocks(step)) {
					// game over
					winner = 1;
					done = true;
					break;
				}
			}
			input1->Update();
			if (input1->LeftPressed()) {
				if (player1_col > 0)
					player1_col--;
			}
			else if (input1->RightPressed()) {
				if (player1_col < PANEL_WIDTH-2)
					player1_col++;
			}
			else if (input1->DownPressed()) {
				if (player1_row < PANEL_HEIGHT-1)
					player1_row++;
			}
			else if (input1->UpPressed()) {
				if (player1_row > 1)
					player1_row--;
			}
			if (input1->ButtonPressed()) {
				panel1->SwapBlocks(player1_row, player1_col, player1_col+1);
			}
			if (input2 != 0) {
				input2->Update();
				if (input2->LeftPressed()) {
					if (player2_col > 0)
						player2_col--;
				}
				else if (input2->RightPressed()) {
					if (player2_col < PANEL_WIDTH-2)
						player2_col++;
				}
				else if (input2->DownPressed()) {
					if (player2_row < PANEL_HEIGHT-1)
						player2_row++;
				}
				else if (input2->UpPressed()) {
					if (player2_row > 1)
						player2_row--;
				}
				if (input2->ButtonPressed()) {
					panel2->SwapBlocks(player2_row, player2_col, player2_col+1);
				}
			}
			if (key[KEY_ESC]) {
				WaitForRelease();
				if (GUI_Prompt("|000255000Q|000255255u|240000240i|255000000t |255255000t|000255000h|000255255i|240000240s |255000000g|255255000a|000255000m|000255255e|240000240?", "Yes", "No")) {
					esc_pressed = true;
					goto end;
				}
			}
		}
		blit(background_bitmap, buffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
		int score = panel1->GetScore();
		panel1->Draw();
		if (input2 == 0) {
			DrawSunkenRectangle(SINGLE_PLAYER_PANEL_START_X-1, PANEL_START_Y-1, SINGLE_PLAYER_PANEL_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(buffer->w/2, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
			int x = SINGLE_PLAYER_PANEL_START_X+(player1_col*BLOCK_SIZE);
			int y = PANEL_START_Y+(player1_row*BLOCK_SIZE)-panel1->GetBottomHeight();
			draw_sprite(buffer, swapper_bitmap, x, y);
		}
		else {
			DrawSunkenRectangle(PANEL1_START_X-1, PANEL_START_Y-1, PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
			int x = PANEL1_START_X+(player1_col*BLOCK_SIZE);
			int y = PANEL_START_Y+(player1_row*BLOCK_SIZE)-panel1->GetBottomHeight();
			draw_sprite(buffer, swapper_bitmap, x, y);
		}
		if (input2 != 0) {
			panel2->Draw();
			DrawSunkenRectangle(PANEL2_START_X-1, PANEL_START_Y-1, PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			score = panel2->GetScore();
			DrawText(PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
			int x = PANEL2_START_X+(player2_col*BLOCK_SIZE);
			int y = PANEL_START_Y+(player2_row*BLOCK_SIZE)-panel2->GetBottomHeight();
			draw_sprite(buffer, swapper_bitmap, x, y);
		}
		BlitToScreen();
		int duration = currentTimeMillis() - last_tick;
		if (duration < TIMESTEP)
			rest(TIMESTEP-duration);
		if (done)
			break;
	}

end:

	if (!esc_pressed) {
		if (input2 && winner >= 1) {
			char *note;
			if (panel1->GetScore() > panel2->GetScore()) {
				if (winner == 1)
					note = "|255255000Player 1 scored higher\n|255255000AND lasted longer!";
				else if (winner == 2)
					note = "|255255000Player 1 scored higher...\n|255255255BUT player 2 lasted longer!";
				else
					note = "|255255000Player 1 scored higher...\n|255255255BUT it was a tie for time!";
			}
			else if (panel1->GetScore() < panel2->GetScore()) {
				if (winner == 1)
					note = "|255255000Player 2 scored higher...\n|255255255BUT player 1 lasted longer!";
				else if (winner == 2)
					note = "|255255000Player 2 scored higher\n|255255000AND lasted longer!";
				else
					note = "|255255000Player 2 scored higher...\n|255255255BUT it was a tie for time!";
			}
			else {
				if (winner == 1)
					note = "|000255000It was a tie for score...\n|255255000BUT player 1 lasted longer!";
				else if (winner == 2)
					note = "|000255000It was a tie for score...\n|255255000BUT player 2 lasted longer!";
				else
					note = "|000255000Wow!\n|000255000A dead-even tie!";
			}
			GUI_WaitMessage(note);
		}
	}

	if (input2 == 0) {
		if (!esc_pressed)
			CheckHighScores(panel1->GetScore());
		delete input1;
		delete panel1;
	}
	else {
		delete input1;
		delete input2;
		delete panel1;
		delete panel2;
	}

	WaitForRelease();

	return esc_pressed;
}
