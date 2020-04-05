#ifndef _LOGIC_H
#define _LOGIC_H

#include "board.h"


enum turn {
    BLACK_NEXT,
    WHITE_NEXT
};

typedef enum turn turn;


enum outcome {
    BLACK_WIN,
    WHITE_WIN,
    DRAW
};

typedef enum outcome outcome;


enum quadrant {
    NW, NE, SW, SE
};

typedef enum quadrant quadrant;

enum direction {
    CW, CCW
};

typedef enum direction direction;


struct game {
    board* b;
    turn next;
};

typedef struct game game;

/* creates new empty game of inputted size and type, white goes first */
game* new_game(unsigned int side, enum type type);

/* frees a game */
void game_free(game* g);

/* places marble in given position according to player's return
if successful place, return 1. if occupied, does nothing & returns 0 */
int place_marble(game* g, pos p);

/* updates board by twisting specificed quadrant in specified direction
flips turn to next player */
void twist_quadrant(game* g, quadrant q, direction d);

/* returns boolean determining whether game is over or not */
int game_over(game* g);

/* returns outcome (black win/ white win/ draw) depending on game state */
outcome game_outcome(game* g);

#endif /* _LOGIC_H */
