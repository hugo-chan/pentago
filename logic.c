#include <stdio.h>
#include <stdlib.h>
#include "logic.h"

game* new_game(unsigned int side, enum type type) {
    game* g = (game*)malloc(sizeof(game));
    if (g == NULL) {
        fprintf(stderr, "new_game: malloc failed.\n");
        exit(1);
    } else {
        g->b = board_new(side, type);
        g->next = WHITE_NEXT; /* WHITE goes first */
        return g;
    }
}

void game_free(game* g) {
    board_free(g->b);
    free(g);
}

int place_marble(game* g, pos p) {
    if ((p.r >= g->b->side) || (p.c >= g->b->side)) {
        fprintf(stderr, "place_marble: position out of bounds.\n");
        exit(1);
    } else if (board_get(g->b, p) != EMPTY) {
        return 0; /* if occupied, do nothing and return false */
    } else if (g->next == WHITE_NEXT) { /* if white's turn */
        board_set(g->b, p, WHITE);
        return 1;
    } else { /* if black's turn */
        board_set(g->b, p, BLACK);
        return 1;
    }
}

/* helper fn: allocate space for a square quadrant matrix of a given size */
square** quadrant_new(unsigned int side) {
    square** new = (square**)malloc(side * sizeof(square*));
    if (new == NULL) {
        fprintf(stderr, "quadrant_new: malloc failed.\n");
        exit(1);
    }
    for (unsigned int i = 0; i < side; i++) {
        new[i] = (square*)malloc(side * sizeof(square));
        if (new[i] == NULL) {
            fprintf(stderr, "quadrant_new: malloc failed.\n");
            exit(1);
        }
    }
    return new;
}

/* helper function for freeing a square quadrant matrix */
void quadrant_free(square** q, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        free(q[i]);
    }
    free(q);
}

/* helper function used to find offset values depending on the quadrant
specified to be twisted, offsets used for matrix insertion & extraction */
void find_offsets(unsigned int* r_offset, unsigned int* c_offset, quadrant q,
unsigned int quad_len) {
    switch (q) { /* initializing offset values for r and c acc. to quadrant */
        case NW:
            *r_offset = 0, *c_offset = 0;
            break;
        case NE:
            *r_offset = 0, *c_offset = quad_len;
            break;
        case SW:
            *r_offset = quad_len, *c_offset = 0;
            break;
        default: /* SE */
            *r_offset = quad_len, *c_offset = quad_len;
            break;
    }
}

void twist_quadrant(game* g, quadrant q, direction d) {
    unsigned int quad_len = g->b->side / 2;
    /* new quadrant matrix used for extracting, twisting, and inserting */
    square** q_new = quadrant_new(quad_len);
    /* declaring matrix indicies & offset values for extraction & insertion */
    unsigned int a = 0, b, r = 0, c = 0, r_offset, c_offset;
    find_offsets(&r_offset, &c_offset, q, quad_len);
    /* extracting quadrant to be twisted */
    for (r = r_offset; r < r_offset + quad_len; r++) { /* iterating row */
        b = 0;
        for (c = c_offset; c < c_offset + quad_len; c++) { /* iterating col */
            q_new[a][b] = board_get(g->b, make_pos(r, c));
            b++;
        }
        a++;
    }
    /* inserting twisted quadrant using derived formulas*/
    if (d == CCW) { /* inserting for counter-clockwise */
        for (r = r_offset; r < r_offset + quad_len; r++) { /* iterating row */
            for (c = c_offset; c < c_offset + quad_len; c++) { /* column */
                //g->b->u.cells[r][c] = q_new[c-c_offset][quad_len+r_offset-1-r];
                square new_sq = q_new[c-c_offset][quad_len+r_offset-1-r];
                board_set(g->b, make_pos(r, c), new_sq);
            }
        }
    } else { /* inserting for clockwise */
        for (r = r_offset; r < r_offset + quad_len; r++) { /* iterating row */
            for (c = c_offset; c < c_offset + quad_len; c++) { /* column */
                //g->b->u.cells[r][c] = q_new[quad_len+c_offset-1-c][r-r_offset];
                square new_sq = q_new[quad_len+c_offset-1-c][r-r_offset];
                board_set(g->b, make_pos(r, c), new_sq);
            }
        }
    }
    g->next = (g->next == WHITE_NEXT) ? BLACK_NEXT : WHITE_NEXT;
    quadrant_free(q_new, quad_len); /* freeing temporary quadrant */
}

/* helper function used to check if side - 1 in a row exists in the board */
int in_a_row(game* g, square tag, unsigned char b, pos p) {
    unsigned int t = 0, checks = g->b->side - 2; /* # of marbles to check */
    for (unsigned int i = 1; i <= checks; i++) {
        if (b == 'h') { /* check horizontal */
            if (tag != board_get(g->b, make_pos(p.r, p.c + i))) {
                t = 1;
                break; /* not consecutive, break loop */
            }
        } else if (b == 'v'){ /* b == 'v', check vertical */
            if (tag != board_get(g->b, make_pos(p.r + i, p.c))) {
                t = 1;
                break; /* not consecutive, break loop */
            }
        } else if (b == 'r'){ /* b == 'r', check right diagonal */
            if (tag!= board_get(g->b, make_pos(p.r + i, p.c + i))) {
                t = 1;
                break;
            }
        } else { /* b == 'l', check left diagonal */
            if (tag!= board_get(g->b, make_pos(p.r + i, p.c - i))) {
                t = 1;
                break;
            }
        }
    }
    return (t == 1) ? 0 : 1; /* not consecutive = return 0 */
}

/* helper function used to check if board is completely filled */
int is_board_full(game* g) {
    for (unsigned int r = 0; r < g->b->side; r++) {
        for (unsigned int c = 0; c < g->b->side; c++) {
            if (board_get(g->b, make_pos(r, c)) == EMPTY) {
                return 0; /* board is not full */
            }
        }
    }
    return 1; /* board is full */
}

/* helper function that returns condition of a game: whether a game is
finished or not. If yes, returns who wins/draw */
int is_game_over(game* g) {
    unsigned int w = 0, b = 0, r, c;
    unsigned char h = 'h', v = 'v', rd = 'r', ld = 'l';
    /* check horizontal */
    for (r = 0; r < g->b->side; r++) {
        for (c = 0; c <= 1; c++) {
            square tag = board_get(g->b, make_pos(r, c));
            if ((tag != EMPTY) && in_a_row(g, tag, h, make_pos(r, c))) {
                (tag == WHITE) ? w++ : b++;
            }
        }
    }
    /* check vertical */
    for (c = 0; c < g->b->side; c++) {
        for (r = 0; r <= 1; r++) {
            square tag = board_get(g->b, make_pos(r, c));
            if ((tag != EMPTY) && in_a_row(g, tag, v, make_pos(r, c))) {
                (tag == WHITE) ? w++ : b++;
            }
        }
    }
    /* check right diagonal: need to check (0,0), (0,1), (1,0), (1,1) */
    for (r = 0; r <= 1; r++) {
        for (c = 0; c <= 1; c++) {
            square tag = board_get(g->b, make_pos(r, c));
            if ((tag != EMPTY) && in_a_row(g, tag, rd, make_pos(r, c))) {
                (tag == WHITE) ? w++ : b++;
            }
        }
    }
    /* left diagonal: need to check (0,len), (0,len-1), (1,len), (1,len-1) */
    for (r = 0; r <= 1; r++) {
        for (c = g->b->side - 2; c <= g->b->side - 1; c++) {
            square tag = board_get(g->b, make_pos(r, c));
            if ((tag != EMPTY) && in_a_row(g, tag, ld, make_pos(r, c))) {
                (tag == WHITE) ? w++ : b++;
            }
        }
    }
    /* who wins? */
    if (w != 0 && b == 0) {
        return 1; /* 1 = white wins */
    } else if (w == 0 && b != 0) {
        return 2; /* 2 = black wins */
    } else if (((w==0 && b==0) && (is_board_full(g)))|| (w!=0 && b!=0)) {
        return 3; /* 3 = draw, board is full + no win or multiple win */
    } else {
        return 0; /* 0 = game is not over */
    }
}

int game_over(game* g) {
    int res = is_game_over(g);
    return (res == 0) ? 0 : 1; /* non-zero means game is over */
}

outcome game_outcome(game* g){
    int res = is_game_over(g);
    switch (res) {
        case 1:
            return WHITE_WIN;
        case 2:
            return BLACK_WIN;
        case 3:
            return DRAW;
        default:
            return DRAW;
    }
}
