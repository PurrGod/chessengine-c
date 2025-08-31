#include <stdio.h>
#include "definitions.h"
#include "search.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "evaluate.h"
#include "bitboard.h"
#include "math.h"

#define infinity 500000

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int negamaxab(Bitboards * bb, int alpha, int beta, int depth) {
    if (depth == 0) {
        return evaluate(bb);
    }

    // generate all pl moves
    moveList list;
    int legal_moves = 0;
    generate_all_moves(bb, bb->side, &list); //seudolegal moves

    if (list.count == 0) {
        int king_sq = ctz(*get_piece_bitboard(bb, bb->kings[bb->side]));
        if (is_square_attacked(bb, king_sq, !bb->side)) {
            return (int)-INFINITY + bb->ply;
        } else {
            return 0; // stalemate
        }
    }

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        make_move(bb, move);

        int king_sq = ctz(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_sq, bb->side)) {

            legal_moves++;
            int score = -negamaxab(bb, -beta, -alpha, depth - 1);

            if (score >= beta) {
                unmake_move(bb);
                return beta;
            }

            alpha = max(alpha, score);


        } 

        unmake_move(bb);
    }
    
    // handle legal and checkmates
    if (legal_moves == 0) {
        int king_sq = ctz(bb->kings[bb->side]);
        if (is_square_attacked(bb, king_sq, !bb->side)){
            return -infinity + bb->ply; 
        } else {return 0;} // stalemate
    }

    return alpha;
}

int find_bestmove(Bitboards * bb, int depth) {
    int bestmove = 0;
    int bestscore = -infinity;
    int alpha = (int)-INFINITY;
    int beta = (int)+INFINITY;

    moveList list;
    generate_all_moves(bb, bb->side, &list); // pseudolegal moves

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        
        // check for legality
        make_move(bb, move);

        int king_sq = ctz(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_sq, bb->side)) {
            int score = -negamaxab(bb, -beta, -alpha, depth - 1);


            if (score > bestscore) {
                bestscore = score;
                bestmove = move;
            }

            alpha = max(score, alpha);
        }

        unmake_move(bb);
    }

    printf("Best move found: %d, score: %d\n", bestmove, bestscore);

    return bestmove;

}
