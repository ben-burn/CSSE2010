/*
 * display.h
 *
 * Authors: Luke Kamols, Jarrod Bennett, Martin Ploschner, Cody Burnett,
 * Renee Nightingale
 */ 

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// Offset for the LED matrix to cater for any game border offset to the edge
// of the LED matrix display.
#define MATRIX_X_OFFSET 2
#define MATRIX_Y_OFFSET 0

// Matrix colour definitions
#define MATRIX_COLOUR_EMPTY		COLOUR_BLACK
#define MATRIX_COLOUR_BORDER	COLOUR_LIGHT_YELLOW
#define MATRIX_COLOUR_PLAYER	COLOUR_GREEN
#define MATRIX_COLOUR_BALL		COLOUR_RED

#define START_SCREEN_BALL_X		(14)
#define START_SCREEN_BALL_Y		(4)

#define PONG_NUM_DYNAMIC_COLS	(3)
#define PONG_DYNAMIC_COL_START	(13)
// Initialise the display for the board, this creates the display
// for an empty board.
void initialise_display(void);

// Shows a starting display.
void show_start_screen(void);

// Update dynamic start screen based on frame number (0-11)
void update_start_screen(uint8_t frame_number);

// Updates the colour at square (x, y) to be the colour
// of the object 'object'.
void update_square_colour(uint8_t x, uint8_t y, uint8_t object);

//uint16_t LED_DIGIT_FONTS[10];


// lmsd
void led_matrix_score(void);
void led_matrix_score_clear(void);


#endif /* DISPLAY_H_ */
