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
// rand() is not a good enough random generator according to statistics
// I don't know the real reason it shall be a topic I will researchc later!
static inline uint64_t splitmix64(uint64_t *state) {
    uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}


// initialize all the hashing keys first
void init_hash_keys() {
    uint64_t seed = 6939946880679460369; // very special seed
    // first initialize piece keys
    for (int piece = wPawn; piece <= bKing; piece++){
        for (int sq = 0; sq < 64; sq++){
            PieceKeys[piece][sq] = splitmix64(&seed);
        }
    }

    // side key
    Sidekey = splitmix64(&seed);

    // castlekeys, there are 16 combinations since 4 bits
    for (int i = 0; i < 16; i++){
        CastleKeys[i] = splitmix64(&seed);
    }

    // unique number for enpassant
    for (int f = 0; f < 8; f++) {
        EPFileKey[f] = splitmix64(&seed);
    }
}

// Using all the initialized hash keys, we shall create a final key using the 
// bitboard properties of a given board state.
U64 zobrist_hashing_posKey(Bitboards * bb) {
    // we take the current boardstate.
    // create a random 64 bit integer.
    U64 key = 0; // this will be our final key for each position

    for (int piece = wPawn; piece <= bKing; piece++) {
        // we iterate through the piece enum, for every "piece" we do a switch statement
        // we do this due to poor planning when designing the bb structure
        // we should have just done bb_pieces[13] so we can easily condense the looop
        // to extract the piece bitboard.
        
        // initialize the bitboard
        U64 piece_bitboard;
        switch(piece){
            // pawns
            case wPawn    : piece_bitboard = bb->pawns[WHITE]; break;
            case bPawn    : piece_bitboard = bb->pawns[BLACK]; break;

            // knights
            case wKnight  : piece_bitboard = bb->knights[WHITE]; break;
            case bKnight  : piece_bitboard = bb->knights[BLACK]; break;

            // bishops
            case wBishop  : piece_bitboard = bb->bishops[WHITE]; break;
            case bBishop  : piece_bitboard = bb->bishops[BLACK]; break;

            // rooks
            case wRook    : piece_bitboard = bb->rooks[WHITE]; break;
            case bRook    : piece_bitboard = bb->rooks[BLACK]; break;

            // queens
            case wQueen   : piece_bitboard = bb->queens[WHITE]; break;
            case bQueen   : piece_bitboard = bb->queens[BLACK]; break;

            // kings
            case wKing    : piece_bitboard = bb->kings[WHITE]; break;
            case bKing    : piece_bitboard = bb->kings[BLACK]; break;
        }

        // we use a while function to pop out the bitboard and 
        // xor it to the key
        while (piece_bitboard) {
            int curr_sq;
            popabit(&piece_bitboard, &curr_sq);
            int pieceRand = PieceKeys[piece][curr_sq];
            key ^= pieceRand;
        }
    }

    // side hash
    // we xor side key if its black turn
    // we determine side by using the # of ply
    // even ply means white, odd is black to move
    if (bb->ply % 2 != 0){
        key ^= Sidekey;
    }

    // xor the en passant
    // do if there is an enpassant
    if (bb->enPas != NO_SQ) {
        int fileEnpas = bb->enPas % 8;
        key ^= EPFileKey[fileEnpas];

    }

    // xor the castle rights
    int castlekeyRand = CastleKeys[bb->castlePerm];
    key ^= castlekeyRand;

    return key;
}