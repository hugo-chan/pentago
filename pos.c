#include <stdio.h>
#include <stdlib.h>
#include "pos.h"

pos make_pos(unsigned int r, unsigned int c) {
    pos p;
    p.r = r;
    p.c = c;
    return p;
}
