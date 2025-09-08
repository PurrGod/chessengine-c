#ifndef HASH_TT
#define HASH_TT

#include "definitions.h"

// struct for hash tables
typedef struct {
    U64 posKey;
    int b_move;
    int score;
    char depth;
    char pv_flag;
} TT_entry;

enum PV_Flags {
    NO_FLAG,            // empty
    FLAG_EXACT,         // beta < score < alpha, fully searched node -> pvNode
    FLAG_LOWER_BOUND,   // score >= beta, too good, cut off -> cutNode
    FLAG_UPPER_BOUND    // score <= alpha, not good enough -> allNode
};

void init_hashtable(int megabytes);
void clear_TT();
TT_entry *probe_TT(U64 poskey);
void store_in_tt(U64 posKey, int depth, int beta, int move, int flag);

#endif