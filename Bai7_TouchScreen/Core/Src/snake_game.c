/*
 * snake_game.c
 *
 *  Created on: Nov 27, 2024
 *      Author: lequo
 */

#include "snake_game.h"

// Global Variables
int snake_x = 120; // Initial snake head position
int snake_y = 120;
int apple_x = 0;
int apple_y = 0;
int direction = RIGHT; // Initial direction

int snake_length = 3;

uint8_t state = START_GAME_INIT;


// State Machine
void game_state_machine() {
    static int elapsed_time = 0; // Tracks elapsed time in milliseconds

    switch (state) {
        case START_GAME_INIT:
            show_start_game_GUI();
            state = START_GAME;
            break;

        case START_GAME:
            if (is_start_button_touched()) {
                state = IN_GAME_INIT;
            }
            break;

        case IN_GAME_INIT:
            show_in_game_static_GUI();
            generate_new_apple();
            reset_snake();
            show_in_game_dynamic_GUI();
            elapsed_time = 0; // Reset elapsed time when entering IN_GAME
            state = IN_GAME;
            break;

        case IN_GAME:
            if (!is_game_over()) {
                elapsed_time += 50; // Increment elapsed time (50ms per call)
                if (elapsed_time >= 1000) { // Check if 1 second has passed
                    elapsed_time = 0; // Reset elapsed time
                    update_snake_position(); // Move the snake
                    check_collisions(); // Check for collisions after the move
                    show_in_game_dynamic_GUI(); // Update game screen
                }
                in_game_logic();
            } else {
                state = GAME_OVER_INIT;
            }
            break;

        case GAME_OVER_INIT:
            show_game_over_GUI();
            state = GAME_OVER;
            break;

        case GAME_OVER:
            if (is_play_again_button_touched()) {
                state = START_GAME_INIT;
            }
            break;
    }
}

// START GAME ========================================================================

void show_start_game_GUI() {
    // Clear screen and display "START GAME" button
    lcd_Clear(WHITE);
//    lcd_StrCenter(0, 145, "START", BLACK, BLACK, 24, 1);
    lcd_ShowStr(90, 148, "START", BLACK, BLACK, 24, 1);
    lcd_DrawRectangle(80, 140, 160, 180, BLACK); // Draw a button around the text
}

// Detect if "START GAME" button is pressed
int is_start_button_touched() {
    touch_Scan();
    if (touch_IsTouched()) {
        int x = touch_GetX();
        int y = touch_GetY();
        if (x > 80 && x < 160 && y > 140 && y < 180) { // Match button dimensions
            return 1;
        }
    }
    return 0;
}

// IN GAME ========================================================================

typedef struct {
    int x;
    int y;
} Segment;

Segment snake_body[400]; // Maximum segments assuming a 240x240 grid

// IN_GAME_INIT
void show_in_game_static_GUI() {
    show_navigation_buttons();
    show_border();
}

void show_navigation_buttons() {
    lcd_DrawRectangle(100, 240, 140, 265, BLACK); // UP
    lcd_ShowStr(113, 245, "UP", BLACK, BLACK, 16, 1);

    lcd_DrawRectangle(100, 290, 140, 315, BLACK); // DOWN
    lcd_ShowStr(104, 295, "DOWN", BLACK, BLACK, 16, 1);

    lcd_DrawRectangle(60, 265, 100, 290, BLACK); // LEFT
    lcd_ShowStr(66, 270, "LEFT", BLACK, BLACK, 16, 1);

    lcd_DrawRectangle(140, 265, 180, 290, BLACK); // RIGHT
    lcd_ShowStr(142, 270, "RIGHT", BLACK, BLACK, 16, 1);
}

void show_border() {
    lcd_DrawRectangle(0, 0, GAME_AREA, GAME_AREA, BLACK); // Draw border
}

// IN_GAME Dynamic GUI
void show_in_game_dynamic_GUI() {
    lcd_Fill(0, 0, GAME_AREA, GAME_AREA, WHITE); // Clear game screen
    draw_snake();
    draw_apple();
}

void draw_snake() {
    for (int i = 0; i < snake_length; i++) {
        lcd_Fill(snake_body[i].x, snake_body[i].y,
                 snake_body[i].x + GRID_SIZE, snake_body[i].y + GRID_SIZE, GREEN);
    }
}

void draw_apple() {
    lcd_Fill(apple_x + 1, apple_y + 1,
             apple_x + GRID_SIZE - 1, apple_y + GRID_SIZE - 1, RED); // Apple slightly smaller
}

// Generate new apple position
void generate_new_apple() {
    apple_x = (rand() % (GAME_AREA / GRID_SIZE)) * GRID_SIZE;
    apple_y = (rand() % (GAME_AREA / GRID_SIZE)) * GRID_SIZE;
}

// Update Snake Position
void update_snake_position() {
    // Shift body segments
    for (int i = snake_length - 1; i > 0; i--) {
        snake_body[i] = snake_body[i - 1];
    }

    // Move head based on direction
    switch (direction) {
        case UP:    snake_y -= GRID_SIZE; break;
        case DOWN:  snake_y += GRID_SIZE; break;
        case LEFT:  snake_x -= GRID_SIZE; break;
        case RIGHT: snake_x += GRID_SIZE; break;
    }

    snake_body[0].x = snake_x;
    snake_body[0].y = snake_y;
}

// Check for collisions
void check_collisions() {
    // Wall collision
    if (snake_x < 0 || snake_x >= GAME_AREA || snake_y < 0 || snake_y >= GAME_AREA) {
        state = GAME_OVER_INIT;
        return;
    }

    // Self-collision
    for (int i = 1; i < snake_length; i++) {
        if (snake_x == snake_body[i].x && snake_y == snake_body[i].y) {
            state = GAME_OVER_INIT;
            return;
        }
    }

    // Apple collision
    if (snake_x == apple_x && snake_y == apple_y) {
        grow_snake();
        generate_new_apple();
    }
}

// Grow snake when apple is eaten
void grow_snake() {
    snake_body[snake_length].x = snake_body[snake_length - 1].x;
    snake_body[snake_length].y = snake_body[snake_length - 1].y;
    snake_length++;
}

// Handle in-game logic
void in_game_logic() {
    // Handle input
    touch_Scan();
    if (touch_IsTouched()) {
        int x = touch_GetX();
        int y = touch_GetY();

        if (x > 100 && x < 140 && y > 240 && y < 265) direction = UP;
        else if (x > 100 && x < 140 && y > 290 && y < 315) direction = DOWN;
        else if (x > 60 && x < 100 && y > 265 && y < 290) direction = LEFT;
        else if (x > 140 && x < 180 && y > 265 && y < 290) direction = RIGHT;
    }

    // Update snake position
//    update_snake_position();

    // Check for collisions
//    check_collisions();

    // Redraw dynamic GUI
//    show_in_game_dynamic_GUI();
}

// GAME OVER ========================================================================

void show_game_over_GUI() {
    // Clear screen and display "GAME OVER" and "PLAY AGAIN" buttons
    lcd_Clear(WHITE);
//    lcd_StrCenter(120, 160, "GAME OVER", WHITE, BLACK, 24, 0);
//    lcd_StrCenter(120, 200, "PLAY AGAIN", WHITE, BLACK, 24, 0);
//    lcd_DrawRectangle(100, 190, 180, 220, BLACK); // Draw a button around the text

    lcd_ShowStr(60, 148, "PLAY AGAIN", BLACK, BLACK, 24, 1);
	lcd_DrawRectangle(50, 140, 190, 180, BLACK); // Draw a button around the text
}

// Detect if "PLAY AGAIN" button is pressed
int is_play_again_button_touched() {
    touch_Scan();
    if (touch_IsTouched()) {
        int x = touch_GetX();
        int y = touch_GetY();
        if (x > 50 && x < 190 && y > 140 && y < 180) { // Match button dimensions
            return 1;
        }
    }
    return 0;
}

void reset_snake() {
    snake_length = 3;
    snake_x = 120;
    snake_y = 120;
    direction = RIGHT;

    for (int i = 0; i < snake_length; i++) {
        snake_body[i].x = snake_x - (i * GRID_SIZE);
        snake_body[i].y = snake_y;
    }
}

int is_game_over() {
    // Wall collision
    if (snake_x < 0 || snake_x >= GAME_AREA || snake_y < 0 || snake_y >= GAME_AREA) {
        return 1;
    }

    // Self-collision
    for (int i = 1; i < snake_length; i++) {
        if (snake_x == snake_body[i].x && snake_y == snake_body[i].y) {
            return 1;
        }
    }

    return 0;
}


