/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 

#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "display.h"
#include "terminalio.h"

//pausegame
#include "serialio.h"

// include for get_current_time
#include "timer0.h"

// rally
#include "ledmatrix.h"

// LED MATRIX SCORE
#include "display.h"

// Seven Seg Display
/* Seven segment display segment values for 0 to 9 */
uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
int8_t seven_seg_cc = 0;

// Player paddle positions. y coordinate refers to lower pixel on paddle.
// x coordinates never change but are nice to have here to use when drawing to
// the display.
static const int8_t PLAYER_X_COORDINATES[] = {PLAYER_1_X, PLAYER_2_X};
static int8_t player_y_coordinates[] = {0, 0};

// Ball position
int8_t ball_x;
int8_t ball_y;

// Ball direction
int8_t ball_x_direction;
int8_t ball_y_direction;

// Player Score
int8_t p1score;
int8_t p2score;

// Player Rally
int8_t p1rally;
int8_t p2rally;

//uint16_t LED_DIGIT_FONTS[10];

int8_t ret_player_1_score(void) {
	return p1score;
}
int8_t ret_player_2_score(void) {
	return p2score;
}

// Randomise Direction
/* Takes current time as the seed, creates a random int 0 or 1, multiplies it by 2, negates it then adds 1
   1) 1 -> 2 -> -2 -> -1
   2) 0 -> 0 -> 0 -> 1 */
void rand_x_direction(void) {
	int8_t rand_int_x;
	rand_int_x = -((rand() & 1) << 1) + 1;
	if (rand_int_x == 1) {
		ball_x_direction = RIGHT;
	} else {
		ball_x_direction = LEFT;
	}
	//ball_x_direction = (rand()%2) ? LEFT : RIGHT;
	
}

void rand_y_direction(void) {
	if ((ball_y != 1) | (ball_y != BOARD_HEIGHT)) {
		ball_y_direction = (rand() % 3) - 1;
	} 
	if (ball_y == BOARD_HEIGHT) {
		ball_y_direction = (rand() % 2) ? 0 : -1;
	}
	if (ball_y == 0) {
		ball_y_direction = (rand() % 2) ? 0 : 1;
	}
	//ball_y_direction = 0;
}

void draw_player_paddle(uint8_t player_to_draw);
void erase_player_paddle(uint8_t player_to_draw);

// Initialise the player paddles, ball and display to start a game of PONG.
void initialise_game(void) {
	
	// initialise the display we are using.
	initialise_display();

	// Start players in the middle of the board
	player_y_coordinates[PLAYER_1] = BOARD_HEIGHT / 2 - 1;
	player_y_coordinates[PLAYER_2] = BOARD_HEIGHT / 2 - 1;

	draw_player_paddle(PLAYER_1);
	draw_player_paddle(PLAYER_2);
	// Player Score
	p1score = 0;
	p2score = 0;
	
	// Rally Counter
	p1rally = 0;
	p2rally = 0;

	// Clear the old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Reset ball position and direction
	ball_x = BALL_START_X;
	ball_y = BALL_START_Y;
	
	srand(get_current_time());
	
	rand_x_direction();
	rand_y_direction();
	/*
	ball_x_direction = LEFT;
	ball_y_direction = UP; */
	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
}

// Draw player 1 or 2 on the game board at their current position (specified
// by the `PLAYER_X_COORDINATES` and `player_y_coordinates` variables).
// This makes it easier to draw the multiple pixels of the players.
void draw_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, PLAYER);
	}
}

// Erase the pixels of player 1 or 2 from the display.
void erase_player_paddle(uint8_t player_to_draw) {
	int8_t player_x = PLAYER_X_COORDINATES[player_to_draw];
	int8_t player_y = player_y_coordinates[player_to_draw];

	for (int y = player_y; y < player_y + PLAYER_HEIGHT; y++) {
		update_square_colour(player_x, y, EMPTY_SQUARE);
	}
}

void move_player_paddle(int8_t player, int8_t direction) {
	 int8_t new_player_position;
	 new_player_position = player_y_coordinates[player] + direction;
	 // Checks if the ball is in the same location as the new_player_position
	 if ((((ball_x == 0) & (ball_y == new_player_position)) | ((ball_x == 0) & (ball_y == new_player_position + 1))) 
	 | (((ball_x == BOARD_WIDTH - 1) & (ball_y == new_player_position)) | ((ball_x == BOARD_WIDTH - 1) & (ball_y == new_player_position + 1)))) {
		// Do Nothing
		;
	 } else {
		 // Allows the player to move as long as the new position does not go out of bounds
		 if ((new_player_position >= 0) & (new_player_position < (BOARD_HEIGHT - 1))) {
			 erase_player_paddle(player);
			 player_y_coordinates[player] = new_player_position;
			 draw_player_paddle(player);
		 }
	 }
}

// Update ball position based on current x direction and y direction of ball
void update_ball_position(void) {
	
	// Determine new ball coordinates
	int8_t new_ball_x = ball_x + ball_x_direction;
	int8_t new_ball_y = ball_y + ball_y_direction;
	if ((new_ball_y < 0) | (new_ball_y > BOARD_HEIGHT - 1)) {
		ball_y_direction *= -1;
		new_ball_y = ball_y + ball_y_direction;
	} else {
	new_ball_y = ball_y + ball_y_direction;
	}
	
	// Reset Ball for wall contact/Scoring
	// ------- RESET PADDLE? --------
	// Player 2 Score
	if ((new_ball_x < 0 )){
		// Reset Ball Properties
		new_ball_x = BALL_START_X;
		new_ball_y = BALL_START_Y;
		/*ball_y_direction = 0;
		ball_x_direction = 0;*/
		rand_x_direction();
		rand_y_direction();
		// Increase Player 2 Score
		p2score += 1;
		move_terminal_cursor(66,10);
		printf_P(PSTR("%d"), p2score);
		// Reset Rally Count
		p1rally = 0;
		p2rally = 0;
		ledmatrix_update_pixel(0, 0, COLOUR_BLACK);
		ledmatrix_update_pixel(15, 0, COLOUR_BLACK);
		for (int8_t y = 0; y += 1;) {
			ledmatrix_update_pixel(0, y, COLOUR_BLACK);
			ledmatrix_update_pixel(15, y, COLOUR_BLACK);
		}
	
	}
	// Player 1 Score
	if ((new_ball_x >= BOARD_WIDTH)) {
		// Reset Ball Properties
		new_ball_x = BALL_START_X;
		new_ball_y = BALL_START_Y;
		/*ball_y_direction = 0;
		ball_x_direction = 1;*/
		rand_x_direction();
		rand_y_direction();
		// Increase Player 1 Score
		p1score += 1;
		move_terminal_cursor(26,10);
		printf_P(PSTR("%d"), p1score);
		// Reset Rally Count
		p1rally = 0;
		p2rally = 0;
		ledmatrix_update_pixel(0, 0, COLOUR_BLACK);
		ledmatrix_update_pixel(15, 0, COLOUR_BLACK);
		for (int8_t y = 0; y += 1;) {
			ledmatrix_update_pixel(0, y, COLOUR_BLACK);
			ledmatrix_update_pixel(15, y, COLOUR_BLACK);
		}
	}
	
	
	// Paddle Bouncin'
	// Player 1
	if ((new_ball_x == PLAYER_X_COORDINATES[0]) & ((new_ball_y == player_y_coordinates[PLAYER_1]) | (new_ball_y == player_y_coordinates[PLAYER_1] + 1))) {
		rand_y_direction();
		ball_x_direction *= -1;
		new_ball_x = ball_x + ball_x_direction;
		new_ball_y = ball_y + ball_y_direction;
		p1rally += 1;
		if (p1rally % 9 != 0) {
			ledmatrix_update_pixel(0, p1rally - 1, COLOUR_RALLY);	
		} else {
			p1rally = 1;
			for (int8_t y = 0; y += 1;) {
				//printf_P(PSTR("%d"), y);
				ledmatrix_update_pixel(0, y, COLOUR_BLACK);	
			}
		}
	}
	// Player 2
	if ((new_ball_x == PLAYER_X_COORDINATES[1]) & ((new_ball_y == player_y_coordinates[PLAYER_2]) | (new_ball_y == player_y_coordinates[PLAYER_2] + 1))) {
		rand_y_direction();
		ball_x_direction *= -1;
		new_ball_x = ball_x + ball_x_direction;
		new_ball_y = ball_y + ball_y_direction;
		p2rally += 1;
		if (p2rally % 9 != 0) {
			ledmatrix_update_pixel(15, p2rally - 1, COLOUR_RALLY);
			} else {
			p2rally = 1;
			for (int8_t y = 0; y += 1;) {
				// int8_t y = 0; y += 1;
				//int8_t y = 0; y < 9; y++
				// why work
				ledmatrix_update_pixel(15, y, COLOUR_BLACK);
			}
		}
	}
	// Erase old ball
	update_square_colour(ball_x, ball_y, EMPTY_SQUARE);
	
	// Assign new ball coordinates
	ball_x = new_ball_x;
	ball_y = new_ball_y;
	
	// Draw new ball
	update_square_colour(ball_x, ball_y, BALL);
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void) {
	if (p1score == 9) {
		seven_seg_dis();
		return 1;
	}
	if (p2score == 9) {
		seven_seg_dis();
		return 1;
	}
	// Detect if the game is over i.e. if a player has won.
	return 0;
}

// Function for SSD
void seven_seg_dis(void) {
	seven_seg_cc = 1 - seven_seg_cc;
	PORTD ^= (1<<2);
	if (seven_seg_cc == 0) {
		PORTC = seven_seg_data[p2score];
		} else {
		PORTC = seven_seg_data[p1score];
	}
}

//uint16_t LED_DIGIT_FONTS[10];


