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

bool SpringShot(void)
{
	input1 = CreateInput(1);
	input1->SetRepeatTimes(150, 150, 300, 400, 150);
	input2 = CreateInput(2);
	if (input2)
		input2->SetRepeatTimes(150, 150, 300, 400, 150);

	if (input2 == 0)
		panel1 = new Panel(SINGLE_PLAYER_PANEL_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
	else {
		panel1 = new Panel(PANEL1_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
		panel2 = new Panel(PANEL2_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
	}

	int player1_column = PANEL_WIDTH/2;
	int player2_column = PANEL_WIDTH/2;
	int player1_block = GetRandomBlock();
	int player2_block = GetRandomBlock();
	int player1_next = GetRandomBlock();
	int player2_next = GetRandomBlock();
	int player1_power = 0;
	int player2_power = 0;
	int player1_charges = START_CHARGES;
	int player2_charges = START_CHARGES;
	int free_charge_count = 0;
	int player1_spring_delay = 0;
	int player2_spring_delay = 0;
	
	int winner = -1;

	int delay_per_block = (int)((float)BLOCK_SIZE / BLAST_INCREMENT);

	bool done = false;
	int last_tick = currentTimeMillis();

	bool esc_pressed = false;

	for (;;) {
		long current_time = currentTimeMillis();
		int steps =  MIN(100, current_time - last_tick);
		last_tick = current_time;
		for (int step = MIN(2, steps); steps > 0; steps -= 2, step = MIN(2, steps)) {
			UpdateFallingBlocks(step);
			if (key[KEY_BACKSPACE])
				SaveScreenshot();
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
			if (input1->LeftPressed() && player1_column)
				player1_column--;
			else if (input1->RightPressed() && player1_column < PANEL_WIDTH-1)
				player1_column++;
			else if (!player1_spring_delay && input1->DownPressed()) {
				Block* block = new Block;
				block->type = player1_block;
				block->popping = false;
				block->pop_count = 0;
				block->y = -BLOCK_SIZE-(BLOCK_SIZE*player1_power);
				block->fall_increment = BLAST_INCREMENT;
				block->fall_count = 0.0;
				block->pop_power = player1_power;
				if (player1_power) {
					play_sample(blast_sample, 255, 0, 1000, 0);
					panel1->AddFallingBlock(player1_column, block, true);
					player1_spring_delay = player1_power * delay_per_block;
					if (!player1_spring_delay) {
						player1_block = player1_next;
						player1_next = GetRandomBlock();
					}
					player1_power = 0;
				}
				else {
					if (!panel1->AddFallingBlock(player1_column, block, false)) {
						play_sample(nocharge_sample, 255, 0, 1000, 0);
						delete block;
					}
					else {
						play_sample(blast_sample, 255, 0, 1000, 0);
						player1_block = player1_next;
						player1_next = GetRandomBlock();
					}
				}
			}
			if (!player1_spring_delay && input1->UpPressed() && player1_power < 3 && player1_charges) {
				play_sample(charge_sample, 255, 0, 1000, 0);
				player1_power++;
				player1_charges--;
			}
			else if (input1->UpPressed())
					play_sample(nocharge_sample, 255, 0, 1000, 0);
			if (player1_spring_delay) {
				player1_spring_delay--;
				if (player1_spring_delay <= 0) {
					player1_block = player1_next;
					player1_next = GetRandomBlock();
				}
				else
					player1_power = player1_spring_delay / delay_per_block;
			}
			if (input2 != 0) {
				input2->Update();
				if (input2->LeftPressed() && player2_column)
					player2_column--;
				else if (input2->RightPressed() && player2_column < PANEL_WIDTH-1)
					player2_column++;
				else if (!player2_spring_delay && input2->DownPressed()) {
					Block* block = new Block;
					block->type = player2_block;
					block->popping = false;
					block->pop_count = 0;
					block->y = -BLOCK_SIZE-(BLOCK_SIZE*player2_power);
					block->fall_increment = BLAST_INCREMENT;
					block->fall_count = 0.0;
					block->pop_power = player2_power;
					if (player2_power) {
						play_sample(blast_sample, 255, 0, 1000, 0);
						panel2->AddFallingBlock(player2_column, block, true);
						player2_spring_delay = player2_power * delay_per_block;
						if (!player2_spring_delay) {
							player2_block = player2_next;
							player2_next = GetRandomBlock();
						}
						player2_power = 0;
					}
					else {
						if (!panel2->AddFallingBlock(player2_column, block, false)) {
							play_sample(nocharge_sample, 255, 0, 1000, 0);
							delete block;
						}
						else {
							play_sample(blast_sample, 255, 0, 1000, 0);
							player2_block = player2_next;
							player2_next = GetRandomBlock();
						}
					}
				}
				if (!player2_spring_delay && input2->UpPressed() && player2_power < 3 && player2_charges) {
					play_sample(charge_sample, 255, 0, 1000, 0);
					player2_power++;
					player2_charges--;
				}
				else if (input1->UpPressed())
						play_sample(nocharge_sample, 255, 0, 1000, 0);
				if (player2_spring_delay) {
					player2_spring_delay--;
					if (player2_spring_delay <= 0) {
						player2_block = player2_next;
						player2_next = GetRandomBlock();
					}
					else
						player2_power = player2_spring_delay / delay_per_block;
				}
			}
			free_charge_count++;
			if (free_charge_count >= FREE_CHARGE_DELAY) {
				free_charge_count = 0;
				player1_charges++;
				player2_charges++;
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
		int player1_spring_x;
		int score = panel1->GetScore();
		if (input2 == 0) {
			player1_spring_x = SINGLE_PLAYER_PANEL_START_X;
			DrawSunkenRectangle(SINGLE_PLAYER_PANEL_START_X-1, PANEL_START_Y-1, SINGLE_PLAYER_PANEL_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(SINGLE_PLAYER_PANEL_START_X, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Score: %d", score);
			DrawText(buffer->w/2, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "%d", player1_charges);
			DrawText(SINGLE_PLAYER_PANEL_START_X+(PANEL_WIDTH*BLOCK_SIZE)-65, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Next:");
			draw_sprite(buffer, block_bitmaps[player1_next], SINGLE_PLAYER_PANEL_START_X+(PANEL_WIDTH*BLOCK_SIZE)-BLOCK_SIZE, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12);
		}
		else {
			player1_spring_x = PANEL1_START_X;
			DrawSunkenRectangle(PANEL1_START_X-1, PANEL_START_Y-1, PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(PANEL1_START_X, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Score: %d", score);
			DrawText(PANEL1_START_X+((PANEL_WIDTH*BLOCK_SIZE)/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "%d", player1_charges);
			DrawText(PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE)-65, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Next:");
			draw_sprite(buffer, block_bitmaps[player1_next], PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE)-BLOCK_SIZE, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12);
		}
		draw_sprite(buffer, spring_bitmaps[player1_power], player1_spring_x+(player1_column*BLOCK_SIZE), PANEL_START_Y-spring_bitmaps[0]->h-BLOCK_SIZE);
		if (!player1_spring_delay)
			draw_sprite(buffer, block_bitmaps[player1_block], player1_spring_x+(player1_column*BLOCK_SIZE), PANEL_START_Y-BLOCK_SIZE-(BLOCK_SIZE*player1_power));
		panel1->Draw();
		if (input2 != 0) {
			DrawSunkenRectangle(PANEL2_START_X-1, PANEL_START_Y-1, PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			score = panel2->GetScore();
			DrawText(PANEL2_START_X, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Score: %d", score);
			DrawText(PANEL2_START_X+((PANEL_WIDTH*BLOCK_SIZE)/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "%d", player2_charges);
			DrawText(PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE)-65, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, false, "Next:");
			draw_sprite(buffer, block_bitmaps[player2_next], PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE)-BLOCK_SIZE, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12);
			draw_sprite(buffer, spring_bitmaps[player2_power], PANEL2_START_X+(player2_column*BLOCK_SIZE), PANEL_START_Y-spring_bitmaps[0]->h-BLOCK_SIZE);
			if (!player2_spring_delay)
				draw_sprite(buffer, block_bitmaps[player2_block], PANEL2_START_X+(player2_column*BLOCK_SIZE), PANEL_START_Y-BLOCK_SIZE-(BLOCK_SIZE*player2_power));
			panel2->Draw();
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
