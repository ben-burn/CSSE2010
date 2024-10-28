/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by <YOUR NAME HERE>
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"


// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Loop forever and continuously play the game.
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	// SSD
	DDRC = 0xFF;
	DDRD |= (1 << 2);
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	show_cursor();
	move_terminal_cursor(10,10);
	printf_P(PSTR("PONG"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 A2 by Benjamin Burn - 45507087"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	uint8_t frame_number = 0;

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S') {
			break;
		}
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}

		current_time = get_current_time();
		if (current_time - last_screen_update > 500) {
			update_start_screen(frame_number);
			frame_number = (frame_number + 1) % 12;
			last_screen_update = current_time;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}



void play_game(void) {
	
	uint32_t last_ball_move_time;
	uint32_t current_time = 0;
	uint8_t btn; // The button pushed
	int32_t game_speed = 500;
	int32_t saved_time;
	// Auto Repeat Button
	int32_t auto_repeat_time;
	int32_t last_button_time = -1;
	int8_t btn_released;
	int8_t button0_held_flag = 0;
	int8_t button1_held_flag = 0;
	int8_t button2_held_flag = 0;
	int8_t button3_held_flag = 0;
	// LED Matrix Score Display
	uint32_t led_matrix_score_time = get_current_time();
	uint32_t led_matrix_score_duration ;
	int8_t old_p1score = ret_player_1_score();
	int8_t old_p2score = ret_player_2_score();
	int8_t new_p1score;
	int8_t new_p2score;
	int8_t break_lms_flag = 0;
	
	last_ball_move_time = get_current_time();
	// Scoring Set-up
	move_terminal_cursor(10,10);
	printf_P(PSTR("Player 1 Score: 0"));
	move_terminal_cursor(50,10);
	printf_P(PSTR("Player 2 Score: 0"));
	move_terminal_cursor(30,5);
	printf_P(PSTR("Game Speed: 500"));
	
	// testing
	/*
	move_terminal_cursor(0,17);
	printf_P(PSTR("led_matrix_score_time"));
	move_terminal_cursor(0,18);
	printf_P(PSTR("get_current_time"));
	move_terminal_cursor(0,19);
	printf_P(PSTR("led_matrix_score_duration"));*/

	// We play the game until it's over
	while (!is_game_over()) {
		// Checks if a button has been pushed. If so, creates a flag that a button is being held. 
		// If flag is 1 paddle moves in given direction while holding button.
		// If btn_released = btn_pushed() then paddle stops moving.
		
		// Auto Repeat Button
		auto_repeat_time = get_current_time() - last_button_time; 
		btn = button_pushed();
		btn_released = button_released();
		// LED Matrix Score Display
		new_p1score = ret_player_1_score();
		new_p2score = ret_player_2_score();
		led_matrix_score_duration = get_current_time() - led_matrix_score_time;
		
		
		
		// works on first score but not second
		if ((new_p1score != old_p1score) | (new_p2score != old_p2score)) {
			led_matrix_score_time = get_current_time();
			old_p1score = new_p1score;
			old_p2score = new_p2score;
			break_lms_flag = 1;
		//}
		
		//if (break_lms_flag == 1) {
			//printf_P(PSTR("%d"), break_lms_flag);
			led_matrix_score_duration = 0;
			led_matrix_score();
		}
		
		// For Testing Purposes
		/*move_terminal_cursor(32,16);
		printf_P(PSTR("%d"), break_lms_flag);
		
		move_terminal_cursor(32,17);
		printf_P(PSTR("%d"), led_matrix_score_time);
		
		move_terminal_cursor(32,18);
		printf_P(PSTR("%d"), get_current_time());
		
		move_terminal_cursor(32,19);
		printf_P(PSTR("%d"), led_matrix_score_duration);*/
		
		if ((break_lms_flag == 1) && (led_matrix_score_duration >= 1500)) {
			led_matrix_score_clear();
			break_lms_flag = 0;
		}
		// If the score is not being displayed on the LED Matrix then the game will take inputs
		if (break_lms_flag == 0) {
			// Ball Movement
			current_time = get_current_time();
			if (current_time >= last_ball_move_time + game_speed) {
				// 500ms (0.5 second) has passed since the last time we move the
				// ball, so update the position of the ball based on current x
				// direction and y direction
				update_ball_position();
				
				// Update the most recent time the ball was moved
				last_ball_move_time = current_time;
			}
			// Button 3
			if ((button3_held_flag == 1) && (auto_repeat_time >= 100)) {
				move_player_paddle(PLAYER_1, UP);
				last_button_time = get_current_time();
			}
			if (btn_released == BUTTON3_PUSHED) {
				button3_held_flag = 0;
			}
			if (btn == BUTTON3_PUSHED) {
				// If button 3 is pushed, move player 1 one space up
				button3_held_flag = 1;
				move_player_paddle(PLAYER_1, UP);
				last_button_time = get_current_time();
			}
			// Button 2
			if ((button2_held_flag == 1) && (auto_repeat_time >= 100)) {
				move_player_paddle(PLAYER_1, DOWN);
				last_button_time = get_current_time();
			}
			if (btn_released == BUTTON2_PUSHED) {
				button2_held_flag = 0;
			}
			if (btn == BUTTON2_PUSHED) {
				// If button 2 is pushed, move player 1 one space down
				button2_held_flag = 1;
				move_player_paddle(PLAYER_1, DOWN);
				last_button_time = get_current_time();
			}
			// Button 1
			if ((button1_held_flag == 1) && (auto_repeat_time >= 100)) {
				move_player_paddle(PLAYER_2, UP);
				last_button_time = get_current_time();
			}
			if (btn_released == BUTTON1_PUSHED) {
				button1_held_flag = 0;
			}
			if (btn == BUTTON1_PUSHED) {
				// If button 1 is pushed, move player 2 one space up
				button1_held_flag = 1;
				move_player_paddle(PLAYER_2, UP);
				last_button_time = get_current_time();
			}
			// Button 0
			if ((button0_held_flag == 1) && (auto_repeat_time >= 100)) {
				move_player_paddle(PLAYER_2, DOWN);
				last_button_time = get_current_time();
			}
			if (btn_released == BUTTON0_PUSHED) {
				button0_held_flag = 0;
			}
			if (btn == BUTTON0_PUSHED) {
				// If button 0 is pushed, move player 1 one space up
				button0_held_flag = 1;
				move_player_paddle(PLAYER_2, DOWN);
				last_button_time = get_current_time();
			}
			
			// Check Serial Inputs
			if (serial_input_available()) {
				char serial_input = fgetc(stdin);
				if ((serial_input == 'w') | (serial_input == 'W')) {
					move_player_paddle(PLAYER_1, UP);
				}
				if ((serial_input == 's') | (serial_input == 'S') | (serial_input == 'd') | (serial_input == 'D')) {
					move_player_paddle(PLAYER_1, DOWN);
				}
				if ((serial_input == 'o') | (serial_input == 'o')) {
					move_player_paddle(PLAYER_2, UP);
				}
				if ((serial_input == 'k') | (serial_input == 'K') | (serial_input == 'l') | (serial_input == 'L')) {
					move_player_paddle(PLAYER_2, DOWN);
				}
				if (serial_input == '1') {
					game_speed = 500;
					move_terminal_cursor(42,5);
					printf_P(PSTR("%d"), game_speed);
				}
				if (serial_input == '2') {
					game_speed = 300;
					move_terminal_cursor(42,5);
					printf_P(PSTR("%d"), game_speed);
				}
				if (serial_input == '3') {
					game_speed = 200;
					move_terminal_cursor(42,5);
					printf_P(PSTR("%d"), game_speed);
				}
				if (serial_input == '4') {
					game_speed = 125;
					move_terminal_cursor(42,5);
					printf_P(PSTR("%d"), game_speed);
				}
				if ((serial_input == 'p') | (serial_input == 'P')) {
					saved_time = current_time;
					move_terminal_cursor(32,50);
					printf_P(PSTR("Game Paused"));
					PORTD ^= (1<<3);
					pause_game();
					PORTD ^= (1<<3);
					last_ball_move_time += (get_current_time() - saved_time);
				}
			
			} // if - serial input
		} //if led_flag == 0
		is_game_over();
	}// main while loop
	// We get here if the game is over.
}

void handle_game_over() {
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
	
	// Do nothing until a button is pushed. Hint: 's'/'S' should also start a
	// new game
	while (button_pushed() == NO_BUTTON_PUSHED) {
		led_matrix_score();
		if (serial_input_available()) {
			char serial_input = fgetc(stdin);
			if ((serial_input == 's') | (serial_input == 'S') | (button_pushed() != NO_BUTTON_PUSHED)) {
				start_screen();
				break;
			}
		}
	}
}

void pause_game(void) {
	while (1) {
		if (serial_input_available()) {
			char serial_input = fgetc(stdin);
			if ((serial_input == 'p') | (serial_input == 'P')) {
				move_terminal_cursor(32,50);
				printf_P(PSTR("           "));
				break;
			}
		}
	}
}



