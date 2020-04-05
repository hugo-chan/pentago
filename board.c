#include <stdio.h>
#include <stdlib.h>
#include "board.h"


board* board_new(unsigned int side, enum type type) {
    board* b = (board*)malloc(sizeof(board));
    if (b == NULL) {
        fprintf(stderr, "board_new: malloc failed.\n");
        exit(1);
    }
    if (side % 2 == 1) { /* board must have side length that is even */
        fprintf(stderr, "board_new: side must be even.\n");
        exit(1);
    }
    if (side < 4) { /* board must have side length of at least four */
        fprintf(stderr, "board_new: side must be at least four.\n");
        exit(1);
    }
    b->side = side;
    b->type = type;
    if (type == CELLS) { /* create cells */
        square** m = (square**)malloc(side * sizeof(square*));
        if (m == NULL) {
            fprintf(stderr, "board_new: malloc failed.\n");
            exit(1);
        }
        for (unsigned int i = 0; i < side; i++) {
            m[i] = (square*)malloc(side * sizeof(square));
            if (m[i] == NULL) {
                fprintf(stderr, "board_new: malloc failed.\n");
                exit(1);
            }
            for (unsigned int j = 0; j < side; j++) {
                m[i][j] = EMPTY; /* cells are intiially empty */
            }
        }
        b->u.cells = m;
    } else if (type == BITS) { /* create bits */
        unsigned int cells = side * side, elements;
        if (cells % 16 == 0) {
            elements = cells / 16; /* each element stores 16 cells */
        } else {
            elements = (cells / 16) + 1; /* round up: store all cells */
        }
        unsigned int* a = (unsigned int*)malloc(elements*sizeof(unsigned int));
        if (a == NULL) {
            fprintf(stderr, "board_new: malloc failed.\n");
            exit(1);
        }
        for (unsigned int i = 0; i < elements; i++) {
            a[i] = 0; /* all cells are empty initially */
        }
        b->u.bits = a;
    } else {
        fprintf(stderr, "board_new: please enter valid type.\n");
        exit(1);
    }
    return b;
}

void board_free(board* b) {
    if (b->type == CELLS) {
        unsigned int rows = b->side, i;
        for (i = 0; i < rows; i++) { /* free each row */
            free(b->u.cells[i]);
        }
    } else { /* type is BITS */
        free(b->u.bits);
    }
    free(b);
}

/* helper function to assign the proper label for headers using ASCII values */
char get_label(unsigned int i) {
    if (i >= 62) {
        return 63;
    } else if (i >= 36) {
        return 61 + i;
    } else if (i >= 10) {
        return 55 + i;
    } else {
        return 48 + i;
    }
}

/* helper function used to print each row of the board */
void print_row(board* b, unsigned int i) {
    unsigned int len = b->side;
    for (unsigned int j = 0; j < len; j++) {
        unsigned char c;
        square s = board_get(b, make_pos(i, j));
        if (s == EMPTY) {
            c = 46; /* ASCII for . */
        } else if (s == BLACK) {
            c = 42; /* ASCII for * */
        } else {
            c = 111; /* ASCII for o */
        }
        if (j == (len / 2) - 1) { /* for blank vertical line */
            printf("%c ", c);
        } else {
            printf("%c", c);
        }
    }
    if (i != len - 1) {
        printf("\n");
    }
}

void board_show(board* b) {
    unsigned int side = b->side, block_len = side / 2, i, j;
    printf("  "); /* beginning of header row */
    for (i = 0; i < side; i++) { /* printing header row */
        unsigned char c = get_label(i);
        if (i == block_len - 1) {
            printf("%c ", c); /* for blank vertical line */
        } else {
            printf("%c", c);
        }
    }
    printf("\n");
    for (j = 0; j < side; j++) { /* printing rows */
        unsigned char c = get_label(j);
        printf("%c ", c);
        print_row(b, j);
        if (j == block_len - 1) {
            printf("\n");
        }
    }
    printf("\n");
}

square board_get(board* b, pos p) {
    if ((p.r >= b->side) || (p.c >= b->side)) {
        fprintf(stderr, "board_get: position out of bounds.\n");
        exit(1);
    }
    if (b->type == BITS) {
        /* cell_num = the cell that pos p points to;
        index = which int in the list the cell's information is in;
        bit_num = which two bits in the int contains the cell's information
        sq = just the two bits containing the cell's information */
        unsigned int cell_num = (p.c * b->side) + p.r;
        unsigned int index = cell_num / 16, bit_num = cell_num % 16;
        unsigned int sq = (b->u.bits[index] >> (bit_num * 2)) & 3;
        switch (sq) {
            case 0:
                return EMPTY;
            case 1:
                return BLACK;
            default: /* sq = 2 */
                return WHITE;
        }
    } else {
        return b->u.cells[p.r][p.c];
    }
}

void board_set(board* b, pos p, square s) {
    if ((p.r >= b->side) || (p.c >= b->side)) {
        fprintf(stderr, "board_set: position out of bounds.\n");
        exit(1);
    }
    if (b->type == BITS) {
        unsigned int sq; /* represents the state of the cell */
        switch (s) {
            case EMPTY:
                sq = 0;
                break;
            case BLACK:
                sq = 1;
                break;
            default:
                sq = 2;
                break;
        }
        /* cell_num, index, bit_num: same definitions as in board_get */
        unsigned int cell_num = (p.c * b->side) + p.r;
        unsigned int index = cell_num / 16, bit_num = cell_num % 16;
        /* strategy: set the two bits first to 00 and then update */
        unsigned int empty_cell = (b->u.bits[index]) & (~(3 << (bit_num * 2)));
        b->u.bits[index] = empty_cell | (sq << (bit_num * 2)); /* update */
    } else { /* type is cells */
        b->u.cells[p.r][p.c] = s;
    }
}
