#include <stdio.h>
#include "definitions.h"
#include "search.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "evaluate.h"
#include "bitboard.h"
#include "math.h"
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

            if (info->stopped == 1) {
                return 0;
            }

            if (score >= beta) {
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

void search_position(Bitboards *bb, SearchInfo *info) {
    int best_move = 0;
    int best_score = -infinity;
    int current_depth;
    
    // Clear stopping flag and nodes counter
    info->stopped = 0;
    info->nodes = 0;

    // Iterative Deepening Loop
    for (current_depth = 1; current_depth <= info->depth; current_depth++) {
        int score = negamaxab(bb, -infinity, infinity, current_depth, info);

        // If the search was stopped, we don't trust the result of the partial search.
        // We use the result from the last fully completed depth instead.
        if (info->stopped == 1) {
            break;
        }

        best_score = score;
        // In a real engine, we'd store the Principal Variation (best line of moves) here.
        // For now, we need to re-search the root to find the move that gives the best score.
        
        moveList list;
        generate_all_moves(bb, bb->side, &list);
        int alpha = -infinity;
        int beta = infinity;

        for (int i = 0; i < list.count; i++) {
            int move = list.moves[i];
            make_move(bb, move);
            int king_sq = ctz(bb->kings[!bb->side]);
            if(!is_square_attacked(bb, king_sq, bb->side)) {
                int temp_score = -negamaxab(bb, -beta, -alpha, current_depth - 1, info);
                if (temp_score > alpha) {
                    alpha = temp_score;
                    best_move = move;
                }
            }
            unmake_move(bb);
        }


        // Print UCI info after each completed depth
        printf("info score cp %d depth %d nodes %llu time %ld\n",
               best_score, current_depth, info->nodes, get_time_ms() - info->starttime);
    }
    
    // --- Send the final best move to the GUI ---
    printf("bestmove ");
    // Helper to print the move in UCI format
    char from_sq_str[3], to_sq_str[3];
    square_to_algebraic(MOVE_FROM(best_move), from_sq_str);
    square_to_algebraic(MOVE_TO(best_move), to_sq_str);
    printf("%s%s", from_sq_str, to_sq_str);

    int promo_piece = MOVE_PROMOTION(best_move);
    if (promo_piece) {
        if (promo_piece == wQueen || promo_piece == bQueen) printf("q");
        else if (promo_piece == wRook || promo_piece == bRook) printf("r");
        else if (promo_piece == wBishop || promo_piece == bBishop) printf("b");
        else if (promo_piece == wKnight || promo_piece == bKnight) printf("n");
    }
    printf("\n");
}
