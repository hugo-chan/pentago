#ifndef _POS_H
#define _POS_H

struct pos {
    unsigned int r, c;
};

typedef struct pos pos;

/* makes a pos structure from inputted row and column */
pos make_pos(unsigned int r, unsigned int c);

#endif /* _POS_H */
