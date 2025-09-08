#include <stdio.h>
#include "definitions.h"
#include "search.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "evaluate.h"
#include "bitboard.h"
#include "hashtable.h"
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


// static int max(int a, int b) {
//     return (a > b) ? a : b;
// }


static void check_time(SearchInfo * info) {
    if (info->timeset == 1 && get_time_ms() > info->stoptime) {
        info->stopped = 1;
    }
}


static void score_and_sort(moveList * list, Bitboards * bb, int hash_move){
    int moves_score[256] = {0};

    for (int i = 0; i < list->count; i++) {
        int move = list->moves[i];

        if (move == hash_move) {
            moves_score[i] = 30000;
        } else if (MOVE_CAPTURED(move) != EMPTY) {
            // score it
            int victim_piece = MOVE_CAPTURED(move);
            int attack_piece = get_piece_on_square(bb, MOVE_FROM(move), bb->side);

            // convert from piece enum to 0-5
            int victim_idx = (victim_piece - 1) % 6;
            int attack_idx = (attack_piece - 1) % 6;

            // add score to capture struct
            moves_score[i] = mvv_lva[victim_idx][attack_idx] + 10000;
        } else {
            moves_score[i] = 0; // quiet move score is 0
        }
    }

    // sorting time
    // everytime we sort move_score, we sort movelist as well
    for (int i = 0; i < list->count; i++) {
        int best_score_index = i;
        for (int j = i + 1; j < list->count; j++) {
            if (moves_score[j] > moves_score[best_score_index]) {
                best_score_index = j;
            }
        }
        // swapping
        int temp_score = moves_score[i];
        moves_score[i] = moves_score[best_score_index];
        moves_score[best_score_index] = temp_score;

        int temp_move = list->moves[i];
        list->moves[i] = list->moves[best_score_index];
        list->moves[best_score_index] = temp_move;
    }

}

typedef struct {
    int move;
    int move_score;
    int flag;
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

static int qsearch(Bitboards * bb, int alpha, int beta, SearchInfo * info) {
    info->nodes++;
    
    int score = evaluate(bb);
    int best_value = score;
    if (best_value >= beta) {return best_value;}
    if (best_value > alpha) {alpha = best_value;}

    moveList list;
    generate_all_moves(bb, bb->side, &list);
    capture_moves cap_list[64];
    int capcount = 0;
    for  (int i = 0; i < list.count; i++) {
        int m = list.moves[i];
        // if move = captures, store it in new array
        if (MOVE_CAPTURED(m) != EMPTY) {
            cap_list[capcount].move = m;
            // score it
            int victim_piece = MOVE_CAPTURED(m);
            int attack_piece = get_piece_on_square(bb, MOVE_FROM(m), bb->side);

            // convert from piece enum to 0-5
            int victim_idx = (victim_piece - 1) % 6;
            int attack_idx = (attack_piece - 1) % 6;

            // add score to capture struct
            cap_list[capcount].move_score = mvv_lva[victim_idx][attack_idx];

            capcount++;
        }
    }

    sort_moves(cap_list, capcount);

    for (int i = 0; i < capcount; i++) {
        make_move(bb, cap_list[i].move);
        int king_sq = ctz(bb->kings[!bb->side]);
        if (is_square_attacked(bb, king_sq, bb->side)) {
            unmake_move(bb);
            continue;
        }
        score = -qsearch(bb, -beta, -alpha, info);
        unmake_move(bb);

        if (score >= beta) {return beta;}
        if (score > alpha) {alpha = score;}
    }
    
    return best_value;

}

int negamaxab(Bitboards * bb, int alpha, int beta, int depth, SearchInfo * info) {
    if ((info->nodes & 2047) == 0) {
        check_time(info);
    }

    if (info->stopped == 1) {return 0;}

    if (bb->ply > 0 && is_repetition(bb)) {return 0;}    


    // 1. Probe the TT for the current position.
    int original_alpha = alpha;
    TT_entry *tt_entry = probe_TT(bb->posKey);
    
    int hash_move = (tt_entry != NULL) ? tt_entry->b_move : 0;
    
    // 2. Check for a TT Cutoff if the entry is valid and deep enough.
    if (tt_entry != NULL) {
        if (tt_entry->depth >= depth){
            if (tt_entry->pv_flag == FLAG_EXACT) {
                return tt_entry->score;
            }
            if (tt_entry->pv_flag == FLAG_LOWER_BOUND && tt_entry->score >= beta) {
                return tt_entry->score;
            }
            if (tt_entry->pv_flag == FLAG_UPPER_BOUND && tt_entry->score <= alpha) {
                return tt_entry->score;
            }
        }
        
    }
    
    if (depth == 0) {return qsearch(bb, alpha, beta, info);}
    
    int best_move_found = 0;

    // generate all pl moves
    moveList list;
    int legal_moves = 0;
    generate_all_moves(bb, bb->side, &list); //pseudolegal moves

    if (list.count == 0) {
        int king_sq = ctz(bb->kings[bb->side]);
        if (is_square_attacked(bb, king_sq, !bb->side)) {
            return (int)-infinity + bb->ply;
        } else {
            return 0; // stalemate
        }
    }

    score_and_sort(&list, bb, hash_move);

    // quiet moves
    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        make_move(bb, move);
        int score;

        int king_sq = ctz(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_sq, bb->side)) {
            legal_moves++;
            if (is_repetition(bb)) {
                unmake_move(bb);

                if (0 > beta) {return beta;}
                if (0 > alpha) {alpha = 0;}
                continue;
            }

            if (i == 0) {
                // full search for the first move (hash move)
                score = -negamaxab(bb, -beta, -alpha, depth - 1, info);
            } else {
                score = -negamaxab(bb, -(alpha + 1), -alpha, depth - 1, info);

                if (score > alpha) {
                    score = -negamaxab(bb, -beta, -alpha, depth - 1, info);
                }

            }

            unmake_move(bb);

            if (info->stopped == 1) {return 0;}
            if (score >= beta) {
                // store in tt
                store_in_tt(bb->posKey, depth, beta, list.moves[i], FLAG_LOWER_BOUND);
                return beta;            
            }

            if (score > alpha) {
                alpha = score;

                best_move_found = move;
            }

        } else {unmake_move(bb);}

    }
    
    // handle legal and checkmates
    if (legal_moves == 0) {
        int king_sq = ctz(bb->kings[bb->side]);
        if (is_square_attacked(bb, king_sq, !bb->side)){
            return -infinity + bb->ply; 
        } else {return 0;} // stalemate
    }

    // At the end of negamaxab, before "return alpha;"

    int final_flag;
    if (alpha <= original_alpha) {
        // If we looped through all the moves and our alpha never improved,
        // it means none of the moves were good. This was an All-Node.
        final_flag = FLAG_UPPER_BOUND;
    } else {
        // If we get here, it means alpha was improved at some point.
        // The score we are returning is a new, exact best score.
        // This was a PV-Node.
        final_flag = FLAG_EXACT;
    }

    // Now, store the result with the correct flag and the best move you found.
    store_in_tt(bb->posKey, depth, alpha, best_move_found, final_flag);

    return alpha;
}

void search_position(Bitboards *bb, SearchInfo *info) {
    int best_move = 0;
    int best_score = 0;
    
    info->stopped = 0;
    info->nodes = 0;    

    for (int current_depth = 1; current_depth <= info->depth; current_depth++) {
        best_score = negamaxab(bb, -infinity, infinity, current_depth, info);

        if (info->stopped) {break;}

        int pvArray[MAX_DEPTH];
        int pv_length = 0;
        TT_entry *entry = probe_TT(bb->posKey);

        while (entry != NULL && entry->b_move != 0 && pv_length < current_depth) {
            pvArray[pv_length++] = entry->b_move;
            make_move(bb, entry->b_move);
            entry = probe_TT(bb->posKey);
            
        }

        for (int i = 0; i < pv_length; i++) {
            unmake_move(bb);
        }

        if (pv_length > 0) {
            best_move = pvArray[0];
        }

        long time_spent = get_time_ms() - info->starttime;
        printf("info score cp %d depth %d nodes %llu time %ld pv ",
               bb->side == WHITE ? best_score : -best_score,
               current_depth,
               info->nodes,
               time_spent > 0 ? time_spent : 1);
        
        for (int i = 0; i < pv_length; i++) {
            print_move_uci(pvArray[i]);
            printf(" ");
        }
        printf("\n");
        fflush(stdout);

    }

    printf("bestmove ");
    if (best_move != 0) {
        print_move_uci(best_move);
    } else {
        // Fallback if no move was found (e.g., immediate mate)
        // Find the first legal move as a fallback.
        moveList fallback_list;
        generate_all_moves(bb, bb->side, &fallback_list);
        for (int i = 0; i < fallback_list.count; i++) {
            make_move(bb, fallback_list.moves[i]);
            if (!is_square_attacked(bb, ctz(bb->kings[!bb->side]), bb->side)) {
                print_move_uci(fallback_list.moves[i]);
                unmake_move(bb);
                break;
            }
            unmake_move(bb);
        }
    }
    printf("\n");
    fflush(stdout);
}


/*

*/