#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "logic.h"

/* helper function that scans user's inputted command-line arguments and
updates the side and type out-parameters 
command-line argument has to be in the form ("-s 4 -c")*/
int find_side_type(int argc, char *argv[], unsigned int* side, enum type* t) {
    int typ = 0; /* pseudo-boolean to see if type is inputted */
    unsigned int count = 0; /* count command line arguments */
    for (unsigned int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            count++;
            if (atoi(argv[i + 1]) < 0) {
                fprintf(stderr, "find_side_type: side length must be "
                "positive.\n");
                exit(1);
            } else {
                *side = atoi(argv[i + 1]);
            }
        } else if (strcmp(argv[i], "-c") == 0) {
            *t = CELLS;
            count++;
            typ++;
        } else if (strcmp(argv[i], "-b") == 0) {
            *t = BITS;
            count++;
            typ++;
        }
    }
    if (count != 2) {
        fprintf(stderr, "find_side_type: please input exactly one side "
        "and one valid type.\n");
        exit(1);
    }
    return typ;
}


/* helper function that creates the game using user-inputted side and type */
game* create_game(int argc, char *argv[]) {
    unsigned int side = 0;
    enum type t;
    unsigned int i = find_side_type(argc, argv, &side, &t);
    if (!side) {
        fprintf(stderr, "create_game: a positive board side length "
        "must be specified.\n");
        exit(1);
    } else if (!i) {
        fprintf(stderr, "create_game: game type must be specified.\n");
        exit(1);
    }
    game* g = new_game(side, t);
    return g;
}

/* helper function that checks if inputted char is valid, if not returns -1
if yes returns the row/column number the char corresponds to */
int char_to_num(char c) {
    if ((c > 47) && (c < 58)) { /* integers */
        return c - 48;
    } else if ((c > 64) && (c < 91)) { /* capital letters */
        return c - 55;
    } else if ((c > 96) && (c < 123)) { /* lower case */
        return c - 61;
    } else {
        return -1;
    }
}

/* helper function that prompts the user to place marble, determines whether
position user inputs is valid. If valid, place marble and return 1, if not,
return 0 */
unsigned int valid_marble_input(game* g) {
    if (g->next == WHITE_NEXT) {
        printf("White: ");
    } else {
        printf("Black: ");
    }
    char row_input, col_input;
    scanf(" %c %c", &row_input, &col_input);
    int r = char_to_num(row_input), c = char_to_num(col_input);
    if ((r == -1) || (c == -1)) {
        printf("Please enter two valid characters.\n");
    } else if ((r >= g->b->side) || (c >= g->b->side)) {
        printf("Please enter numbers within the board size.\n");
    } else if (board_get(g->b, make_pos((unsigned int)r, (unsigned int)c))
              != EMPTY) {
        printf("Position already occupied.\n");
    } else {
        place_marble(g, make_pos((unsigned int)r, (unsigned int)c));
        return 1;
    }
    return 0;
}

/* message printed before prompting user to twist quadrant, done separately
to avoid message being called repeatedly if user enters invalid input */
void prompt_twist_message() {
    printf("Twist quadrant:\nEnter a quadrant (NW, NE, SW, SE) "
    "followed by a twist direction (0 for counter-clockwise, 1 for clockwise)"
    " with no spaces in between. Your input should be three characters long. "
    "\nFor example, enter 'NW1' to twist the northwest quadrant clockwise or"
    " 'SW0' to twist the southwest quadrant counterclockwise: ");
}

/* helper function that prompts the user to twist quadrant, determines whether
quadrant and direction that user inputs is valid. If valid, twist the
specified quadrant in the specified direction and return 1, if not, return 0 */
unsigned int prompt_twist(game* g) {
    char s[4];
    scanf(" %s", s);
    direction d;
    if (s[3] != '\0') {
        printf("Please enter a valid input with no spaces (e.g. NW0).\n");
        return 0;
    }
    if (s[2] == '0') {
        d = CCW;
    } else if (s[2] == '1') {
        d = CW;
    } else {
        printf("Please enter a valid input with no spaces (e.g. NW0).\n");
        return 0;
    }
    if ((s[0] == 'N') && (s[1] == 'W')) {
        twist_quadrant(g, NW, d);
    } else if ((s[0] == 'N') && (s[1] == 'E')) {
        twist_quadrant(g, NE, d);
    } else if ((s[0] == 'S') && (s[1] == 'W')) {
        twist_quadrant(g, SW, d);
    } else if ((s[0] == 'S') && (s[1] == 'E')) {
        twist_quadrant(g, SE, d);
    } else {
        printf("Please enter a valid input with no spaces (e.g. NW0).\n");
        return 0;
    }
    return 1;
}

/* helper function called before and after twist to see check if game is over.
variable i used to keep track whether function call is before or after twist
i is 0 before twist and 1 after twist */
void check_game_state(game* g, unsigned int i) {
    if (game_over(g)) {
        if ((game_outcome(g) == DRAW) && i) { /* DRAW and after twist */
            printf("Game over. Draw.\n");
                board_show(g->b);
                game_free(g);
                exit(1);
        } else if (game_outcome(g) == WHITE_WIN) {
            printf("Game over. White wins.\n");
            board_show(g->b);
            game_free(g);
            exit(1);
        } else if (game_outcome(g) == BLACK_WIN) {
            printf("Game over. Black wins.\n");
            board_show(g->b);
            game_free(g);
            exit(1);
        }
    }
    /* game is not over or game is over but full board and before twist */
    if (!i) {
        unsigned int j = 0;
        board_show(g->b);
        prompt_twist_message();
        while (!j) { /* prompts user to input until valid & quadrant twisted */
            j = prompt_twist(g);
        }
    }
}

/* main function that is run, uses helper functions defined above to run
the game, exits when game is over */
int main(int argc, char *argv[]) {
    game* g = create_game(argc, argv);
    printf("Welcome to the game!\nThe current board state is:\n");
    while (1) { /* run loop until game exits by itself */
        board_show(g->b); /* draw state of the board*/
        unsigned int i = 0;
        while (!i) { /* prompts user to input until valid and marble placed */
            i = valid_marble_input(g);
        }
        check_game_state(g, 0);
        check_game_state(g, 1); /* check game state after twist */
    }
}
