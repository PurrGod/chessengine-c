#include <stdio.h>
#include "definitions.h"
#include "search.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "evaluate.h"
#include "bitboard.h"

#define INFINITY 500000

static int negamaxab(Bitboards * bb, int depth) {
    if (depth == 0) {
        return evaluate(bb);
    }

    // generate all pl moves
    moveList list;
    int legal_moves;
    generate_all_moves(bb, bb->side, &list); //seudolegal moves

    // initialize best score to infinity
    int maxScore = -INFINITY;

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];

        make_move(bb, move);
        int king_sq = ctz(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_sq, bb->side)) {
            legal_moves++;
            int score = -negamaxab(bb, depth - 1);
            if (score > maxScore) {maxScore = score;}
        }

        unmake_move(bb);
    }
    
    // handle legal and checkmates
    if (list.count == 0) {
        int king_sq = ctz(bb->kings[bb->side]);
        if (is_square_attacked(bb, king_sq, !bb->side)){
            return -INFINITY + bb->ply; 
        } else {return 0;} // stalemate
    }

    return maxScore;
}

int find_bestmove(Bitboards * bb, int depth) {
    int bestmove = 0;
    int bestscore = -INFINITY;

    moveList list;
    generate_all_moves(bb, bb->side, &list); // pseudolegal moves

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        
        // check for legality
        make_move(bb, move);

        int king_sq = ctz(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_sq, bb->side)) {
            int score = -negamaxab(bb, depth - 1);
            if (score > bestscore) {score = bestscore;}
        }

        unmake_move(bb);
    }

    printf("Best move found: %d, score: %d\n", bestmove, bestscore);

    return bestmove;

}
