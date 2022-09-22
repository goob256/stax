#include "stax.h"

bool Sucker(void)
{
	input1 = CreateInput(1);
	input1->SetRepeatTimes(150, 150, 0, 150, 150);
	input2 = CreateInput(2);
	if (input2)
		input2->SetRepeatTimes(150, 150, 0, 150, 150);

	if (input2 == 0)
		panel1 = new Panel(SINGLE_PLAYER_PANEL_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
	else {
		panel1 = new Panel(PANEL1_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
		panel2 = new Panel(PANEL2_START_X, PANEL_START_Y, configuration.initial_height, RISE_START);
	}

	int player1_column = PANEL_WIDTH/2;
	int player2_column = PANEL_WIDTH/2;
	int player1_block = -1;
	int player2_block = -1;

	int winner = -1;

	bool sucking = false;
	bool suck_sample_playing = false;

	bool done = false;
	int last_tick = currentTimeMillis();

	bool esc_pressed = false;

	for (;;) {
		long current_time = currentTimeMillis();
		int steps = MIN(100, current_time - last_tick);
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
			sucking = false;
			input1->Update();
			if (input1->LeftPressed() && player1_column) {
				if (input1->UpPressed())
					panel1->shiftTopFallingBlockLeft(player1_column);
				player1_column--;
			}
			else if (input1->RightPressed() && player1_column < PANEL_WIDTH-1) {
				if (input1->UpPressed())
					panel1->shiftTopFallingBlockRight(player1_column);
				player1_column++;
			}
			else if (input1->DownPressed()) {
				Block* block = new Block;
				block->type = player1_block;
				block->popping = false;
				block->pop_count = 0;
				block->y = -BLOCK_SIZE;
				block->fall_increment = FALL_INCREMENT;
				block->fall_count = 0.0;
				block->pop_power = 0;
				if (panel1->AddFallingBlock(player1_column, block, false))
					player1_block = -1;
				else
					delete block;
			}
			if (input1->UpPressed() && player1_block < 0) {
				sucking = true;
				Block* block = panel1->GetTopFallingBlock(player1_column);
				if (block == 0) {
					panel1->FreeTopLandedBlock(player1_column);
					block = panel1->GetTopFallingBlock(player1_column);
				}
				else if (block->y == BLOCK_MINIMUM_Y) {
					player1_block = block->type;
					panel1->RemoveTopFallingBlock(player1_column);
					play_sample(thunk_sample, 255, 0, 1000, 0);
				}
				if (block != 0) {
					block->fall_increment = SUCK_INCREMENT;
				}
				
			}
			if (input2 != 0) {
				input2->Update();
				if (input2->LeftPressed() && player2_column) {
					if (input2->UpPressed())
						panel2->shiftTopFallingBlockLeft(player2_column);
					player2_column--;
				}
				else if (input2->RightPressed() && player2_column < PANEL_WIDTH-1) {
					if (input2->UpPressed())
						panel2->shiftTopFallingBlockRight(player2_column);
					player2_column++;
				}
				else if (input2->DownPressed()) {
					Block* block = new Block;
					block->type = player2_block;
					block->popping = false;
					block->pop_count = 0;
					block->y = -BLOCK_SIZE;
					block->fall_increment = FALL_INCREMENT;
					block->fall_count = 0.0;
					if (panel2->AddFallingBlock(player2_column, block, false))
						player2_block = -1;
					else
						delete block;
				}
				if (input2->UpPressed() && player2_block < 0) {
					sucking = true;
					Block* block = panel2->GetTopFallingBlock(player2_column);
					if (block == 0) {
						panel2->FreeTopLandedBlock(player2_column);
						block = panel2->GetTopFallingBlock(player2_column);
					}
					else if (block->y == BLOCK_MINIMUM_Y) {
						player2_block = block->type;
						panel2->RemoveTopFallingBlock(player2_column);
						play_sample(thunk_sample, 255, 0, 1000, 0);
					}
					if (block != 0) {
						block->fall_increment = SUCK_INCREMENT;
					}
					
				}
			}
			if (sucking == true) {
				if (suck_sample_playing == false) {
					play_sample(suck_sample, 255, 0, 1000, 1);
					suck_sample_playing = true;
				}
			}
			else {
				stop_sample(suck_sample);
				suck_sample_playing = false;
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
		int player1_magnet_x;
		int score = panel1->GetScore();
		if (input2 == 0) {
			player1_magnet_x = SINGLE_PLAYER_PANEL_START_X;
			DrawSunkenRectangle(SINGLE_PLAYER_PANEL_START_X-1, PANEL_START_Y-1, SINGLE_PLAYER_PANEL_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(buffer->w/2, PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
		}
		else {
			player1_magnet_x = PANEL1_START_X;
			DrawSunkenRectangle(PANEL1_START_X-1, PANEL_START_Y-1, PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			DrawText(PANEL1_START_X+(PANEL_WIDTH*BLOCK_SIZE/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
		}
		draw_sprite(buffer, magnet_bitmap, player1_magnet_x+(player1_column*BLOCK_SIZE), PANEL_START_Y-magnet_bitmap->h-BLOCK_SIZE);
		if (player1_block >= 0) {
			draw_sprite(buffer, block_bitmaps[player1_block], player1_magnet_x+(player1_column*BLOCK_SIZE), PANEL_START_Y-BLOCK_SIZE);
		}
		panel1->Draw();
		if (input2 != 0) {
			DrawSunkenRectangle(PANEL2_START_X-1, PANEL_START_Y-1, PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE));
			draw_sprite(buffer, magnet_bitmap, PANEL2_START_X+(player2_column*BLOCK_SIZE), PANEL_START_Y-magnet_bitmap->h-BLOCK_SIZE);
			score = panel2->GetScore();
			DrawText(PANEL2_START_X+(PANEL_WIDTH*BLOCK_SIZE/2), PANEL_START_Y+(PANEL_HEIGHT*BLOCK_SIZE)+12, true, "Score: %d", score);
			if (player2_block >= 0)
				draw_sprite(buffer, block_bitmaps[player2_block], PANEL2_START_X+(player2_column*BLOCK_SIZE), PANEL_START_Y-BLOCK_SIZE);
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

