#ifndef TICTACTOEFUNCTIONS_H
#define TICTACTOEFUNCTIONS_H

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

char** init_board(); // creates the board
void update_board(char** board, char player, char row, char col); // will place an X or O on position if valid
uint16_t check_victory(char** board, char player); // checks if game is a victory for either player using bit masks
void remake_board(char** board); // remakes the game
uint16_t get_player_positions(char** board, char player); // returns digital readings of a player's positions on the board
uint8_t find_changed_bit(uint16_t old_input, uint16_t new_input); // determine the bit position of the changed bit 
void display_board(char** board); // prints board to console

#endif