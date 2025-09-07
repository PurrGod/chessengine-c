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

// mvv-lva tables:
// [victim][attacker] piecetype - 1
// score is victim - attacker
//    higher score means attacker piece is of low value
//    negative score means attack piece is higher value than victim piece
static const int mvv_lva[5][5] = {
  {   1500,  1290,   1250,   1100,   700 }, // Victim: Pawn
  {   4860,  4650,   4610,   4460,  4060 }, // Victim: Knight
  {   5500,  5290,   5250,   5100,  4700 }, // Victim: Bishop
  {   7900,  7690,   7650,   7500,  7100 }, // Victim: Rook
  {  14300, 14090,  14050,  13900, 13500 }, // Victim: Queen
};



typedef struct {
    int move;
    int move_score;
} capture_moves;

// do inplace insertion sort
static void sort_moves(capture_moves * list, int count) {
    for (int i = 1; i < count; i++) {
        capture_moves key = list[i];
        int j = i - 1;

        while (j >= 0 && list[j].move_score < key.move_score) {
            list[j + 1] = list[j];
            j = j - 1;
        }

        list[j + 1] = key;

    }
}

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

    // MVV - LVA process
    capture_moves capList[64];
    int capcount = 0;
    for  (int i = 0; i < list.count; i++) {
        int m = list.moves[i];
        // if move = captures, store it in new array
        if (MOVE_CAPTURED(m) != EMPTY) {
            capList[capcount].move = m;
            // score it
            int victim_piece = MOVE_CAPTURED(m);
            int attack_piece = get_piece_on_square(bb, MOVE_FROM(m), bb->side);

            // convert from piece enum to 0-5
            int victim_idx = (victim_piece - 1) % 6;
            int attack_idx = (attack_piece - 1) % 6;

            // add score to capture struct
            capList[capcount].move_score = mvv_lva[victim_idx][attack_idx];

            capcount++;
        }
    }

    // sort the moves
    sort_moves(capList, capcount);

    // call negamax on these if legal
    for (int i = 0; i < capcount; i++) {
        int m = capList[i].move;
        make_move(bb, m);

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


    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        // check if move is capture: skip it since we already checked those early
        if (MOVE_CAPTURED(move) != EMPTY) {
            continue;
        }
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

    // fall back when time runs out, just play a legal move
    // Fallback to find the first legal move
    moveList initial_list;
    generate_all_moves(bb, bb->side, &initial_list);
    for (int i = 0; i < initial_list.count; i++) {
        int move = initial_list.moves[i];
        make_move(bb, move);
        if(!is_square_attacked(bb, ctz(bb->kings[!bb->side]), bb->side)) {
            best_move = move;
            unmake_move(bb);
            break;
        }
        unmake_move(bb);
    }
    

    for (current_depth = 1; current_depth <= info->depth; current_depth++) {
        printf("--- Depth %d ---\n", current_depth);

        // This loop finds the best move for the depth we just completed
        moveList list;
        generate_all_moves(bb, bb->side, &list);
        int alpha = -infinity;
        int beta = infinity;
        int iteration_best_move = best_move; // Use a temporary variable for this depth

        for (int i = 0; i < list.count; i++) {
            int move = list.moves[i];
            U64 nodes_before_move = info->nodes;
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
            U64 nodes_for_this_move = info->nodes - nodes_before_move;
            print_move_uci(move);
            printf(": %llu nodes\n", nodes_for_this_move);

        }
    
        // Only update the official best_move and score if the search for this
        // depth completed without being interrupted.
        if (info->stopped == 0) {
            best_move = iteration_best_move;
            best_score = alpha;

            int uci_score = best_score;
            if (bb->side == BLACK) { uci_score = -best_score; }

            printf("info score cp %d depth %d nodes %llu time %ld\n",
                   uci_score, current_depth, info->nodes, get_time_ms() - info->starttime);
        } else {
            // If the search was stopped, break out of the main loop immediately.
            // Do NOT update best_move, preserving the result from the last completed depth.
            // printf("Search stopped, fallback to the best move from depth %d, ", current_depth - 1);
            break;
        }
    }

    if (best_move == 0) {
        return;
    }
    
    printf("bestmove ");
    print_move_uci(best_move);
    printf("\n");
}