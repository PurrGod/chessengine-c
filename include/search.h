#ifndef SEARCH_H
#define SEARCH_H

#include "definitions.h"


typedef struct {
    int move;
    int move_score;
} capture_moves;


void search_position(Bitboards *bb, SearchInfo *info);

#endif