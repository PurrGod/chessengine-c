#ifndef SEARCH_H
#define SEARCH_H

#include "definitions.h"

static int negamaxab(Bitboards * bb, int depth);

int find_bestmove(Bitboards * bb, int depth);

#endif