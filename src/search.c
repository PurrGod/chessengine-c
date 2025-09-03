#include <stdio.h>
#include "definitions.h"
#include "search.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "evaluate.h"
#include "bitboard.h"
#include "uci.h"
#include "time.h"

#define infinity 100000


static int max(int a, int b) {
    return (a > b) ? a : b;
}


static void check_time(SearchInfo * info) {
    if (info->timeset == 1 && get_time_ms() > info->stoptime) {
        info->stopped = 1;
    }
}

int negamaxab(Bitboards * bb, int alpha, int beta, int depth, SearchInfo * info) {
    if ((info->nodes & 2047) == 0) {
        check_time(info);
    }

    if (info->stopped == 1) {
        return 0;
    }


    if (depth == 0) {
        info->nodes++;
        return evaluate(bb);
    }

    // generate all pl moves
    moveList list;
    int legal_moves = 0;
    generate_all_moves(bb, bb->side, &list); //seudolegal moves

    if (list.count == 0) {
        int king_sq = ctz(bb->kings[bb->side]);
        if (is_square_attacked(bb, king_sq, !bb->side)) {
            return (int)-infinity + bb->ply;
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
            int score = -negamaxab(bb, -beta, -alpha, depth - 1, info);
            unmake_move(bb);

            if (info->stopped == 1) {return 0;}
            if (score >= beta) {return beta;}
            alpha = max(alpha, score);

        } else {unmake_move(bb);}

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

void search_position(Bitboards *bb, SearchInfo *info) {
    int best_move = 0;
    int best_score = -infinity;
    int current_depth;
    
    info->stopped = 0;
    info->nodes = 0;

    for (current_depth = 1; current_depth <= info->depth; current_depth++) {
        // If the search was stopped, don't trust the results of the second search loop.
        // The existing best_move from the previous completed depth is more reliable.
        if (info->stopped == 1) {
            break;
        }
        
        // This loop finds the best move for the depth we just completed
        moveList list;
        generate_all_moves(bb, bb->side, &list);
        int alpha = -infinity;
        int beta = infinity;
        int iteration_best_move = best_move; // Use a temporary variable for this depth

        for (int i = 0; i < list.count; i++) {
            int move = list.moves[i];
            make_move(bb, move);
            int king_sq = ctz(bb->kings[!bb->side]);
            if(!is_square_attacked(bb, king_sq, bb->side)) {
                int temp_score = -negamaxab(bb, -beta, -alpha, current_depth - 1, info);
                if (temp_score > alpha) {
                    alpha = temp_score;
                    iteration_best_move = move; // Only update the temporary variable
                }
            }
            unmake_move(bb);
        }

        // The search for this depth completed without being stopped,
        // so we can now safely update our main best_move.
        best_move = iteration_best_move;
        best_score = alpha; // alpha is usually the highest score.

        int uci_score = best_score;
        if (bb->side == BLACK) {uci_score = -best_score;}

        printf("info score cp %d depth %d nodes %llu time %ld\n",
               uci_score, current_depth, info->nodes, get_time_ms() - info->starttime);
    }

    if (best_move == 0) {
        return;
    }
    
    printf("bestmove ");
    print_move_uci(best_move);
    printf("\n");
}