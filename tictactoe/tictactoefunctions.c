#include "tictactoefunctions.h"

/* Initializes a 3x3 board as a 2d array and returns address */
char** init_board() {
    char** board = (char**) malloc(3 * sizeof(char*));
    uint8_t i, j;
    for(i = 0; i < 3; i++) {
        board[i] = (char*) malloc(3 * sizeof(char));
        for(j = 0; j < 3; j++) {
            board[i][j] = '-';
        }
    }

    return board;
}


/* Clears the board */
void remake_board(char** board) {
    uint8_t i, j;

    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            board[i][j] = '-'; // initialize board to clear array
        }
    }
} 

/* Displays board on console w/ grid */
void display_board(char** board) {
   uint8_t i, j;

   printf(" A B C\r\n");

   for(i = 0; i < 3; i++) {
       printf("%d ", i + 1);
       for(j = 0; j < 3; j++) {
           printf("%c ", board[i][j]);
       }
       printf("\r\n");
   } 
}

/* Returns a digital 16 bit reading of the given player's positions on the board.
 * If the player has a tile in a given position, it is written as a 1.
 * The two position data bytes are represented as follows:
 * xxxxxxxC3B3A3C2B2A2C1B1A1
 */
uint16_t get_player_positions(char** board, char player) {
    assert(player == 'x' || player == 'o');
    uint8_t i, j;
    uint16_t position_data = 0;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            if(board[i][j] == player) {
                position_data |= (1 << (i * 3 + j)); // shifts in a 1 to the appropriate position
            }
        }
    }

    return position_data;
}


/*
 * Given an old and a new 16 bit input, returns the position of the changed bit
 * There are four cases to check:
 * Case 1: The two inputs are equal 
 * Case 2: A single bit flips from a 0 to a 1
 * Case 3: More than one bit flips from a 0 to a 1
 * Case 4: One or more bits flip from a 1 to a 0
 * 
 * Returns 0x0A if inputs are equal, 0x0B if more than one bit flips, 0x0C if a bit goes from 1 to 0, pos of changed bit otherwise
 */
uint8_t find_changed_bit(uint16_t old_input, uint16_t new_input) {
    // case 1
    if(old_input == new_input) {
        return 0x0A;
    }

    uint16_t bit_mask = 0, changed = 0, old_bit = 0, new_bit = 0;
    uint8_t i, j, pos, flag = 0;
    for(i = 0; i < 3; i++) {
        for(j = 0; j < 3; j++) {
            pos = i * 3 + j;
            bit_mask = (1 << (i * 3 + j));
            old_bit = old_input & bit_mask;
            new_bit = new_input & bit_mask;

            if(!(new_bit - bit_mask)) {

                if(old_bit - bit_mask) {
                        // case 2 or 3
                        if(!flag) {
                            flag = 1;
                            changed = pos; 
                        }
                        else return 0x0B;
                }
            } else {
                
                if(!(old_bit - bit_mask)) {
                    // case 4
                    return 0x0C;
                }
            }
        }
    }

    return changed;
}


/* Places x or o on given board tile if valid */
void update_board(char** board, char player, char row, char col) {
    assert(row >= 0 && row < 3 && col >= 0 && col < 3);
    assert(player == 'x' || player == 'o');
    board[row][col] = player;
}

/* Checks if a given player has won the game, using bit masks.
 * If the player has won, returns the specific 16 bit victory condition that the player achieved.
 * Otherwise, returns 0
 */
uint16_t check_victory(char** board, char player) {
    
   // get_player_positions will return 2 bytes in format of xxxxxxxC3B3A3C2B2A2C1B1A1
   // The following combinations will win the game if all 1's are from the same player:
   // 111xxxxxx, xxx111xxx, xxxxxx111, 1xx1xx1xx, x1xx1xx1x, xx1xx1xx1, xx1x1x1xx, 1xxx1xxx1

    const uint16_t conditions[8] = {0b0000000111000000, 0b0000000000111000, 0b0000000000000111, 0b0000000100100100,
                           0b0000000010010010, 0b0000000001001001, 0b0000000001010100, 0b0000000100010001};

    uint16_t positions = get_player_positions(board, player);
    uint8_t i;

    for(i = 0; i < 8; i++) {
        if(!((positions & conditions[i]) - conditions[i])) {
            return conditions[i];
        }
    }

   return 0;
}