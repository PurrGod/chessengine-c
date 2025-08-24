#include "definitions.h"
#include "stdlib.h"

//generate position keys to create a unique key for each position
//this is used to check for repeated states
//common technique is to XOR all te keys

U64 PieceKeys[13][64]; // keys for each piece wPawn to bKing 0 = empty, 1 - 13
U64 Sidekey;
U64 CastleKeys[16];
U64 EPFileKey[8]; // enpassant file key

// generate 64 bit random integer
// rand()

// initialize all the hashing keys first
static void init_hash_keys() {
    // first initialize piece keys
    for (int piece = wPawn; piece <= bKing; piece++){
        for (int sq = 0; p < 64; p++){
            PieceKeys[piece][sq] = rand();
        }
    }

    // side key
    Sidekey = rand();

    // castlekeys, there are 16 combinations since 4 bits
    for (int i = 0; i < 16; i++){
        CastleKeys[i] = rand();
    }

    // unique number for enpassant
    for (int f = 0; f < 8; f++) {
        EPFileKey[f] = rand();
    }
}

U64 zobrist_hashing(Bitboards * bb) {
    // we take the current boardstate.
    // create a random 64 bit integer.
    U64 key = {0};


}