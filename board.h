#ifndef _BOARD_H
#define _BOARD_H

#include "pos.h"


enum square {
    EMPTY,
    BLACK,
    WHITE
};

typedef enum square square;


union board_rep {
    enum square** cells;
    unsigned int* bits;
};

typedef union board_rep board_rep;

enum type {
    CELLS, BITS
};


struct board {
    unsigned int side;
    enum type type;
    board_rep u;
};

typedef struct board board;

/* constructs an empty board of inputted size (must be even) */
board* board_new(unsigned int side, enum type type);

/* frees an inputted board, also frees the two dimensional squares array for
CELLS, frees the unsigned int array for BITS */
void board_free(board* b);

/* prints out an inputted board, displaying the marbles and rows/columns */
void board_show(board* b);

/* returns the state (tag) of a certain cell in the board */
square board_get(board* b, pos p);

/* sets a certain cell in the board to a specified state (tag) */
void board_set(board* b, pos p, square s);

#endif /* _BOARD_H */
