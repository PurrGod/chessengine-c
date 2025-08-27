#include <stdio.h>
#include "definitions.h"

// using C's implicit ASCII conversion we can use the calculations from 
// ranks and files to get the notation 
// Convert a square index (0-63) to algebraic notation (e.g., 0 -> "a1")
void square_to_algebraic(int square, char *notation) {
    notation[0] = (char)('a' + (square % 8));
    notation[1] = (char)('1' + (square / 8));
    notation[2] = '\0';
}

// Convert algebraic notation (e.g., "a1") to square index (0-63)
int algebraic_to_square(const char *square) {
    int file = square[0] - 'a';
    int rank = square[1] - '1';
    return rank * 8 + file;
}

// Function to extract indices of set bits and return the count
// move generations and board analysis
int popbits(U64  bb, int *indices) {
    int count = 0;
    while (bb) {
        indices[count++] = __builtin_ctzll(bb); // Get the index of the least significant set bit
        bb &= bb - 1; // Clear the least significant set bit
    }
    return count;
}

//isolate the lsb
//invert it with complement and add one
//take the xor to flip the lsb
U64 popabit(U64 *bb, int *lsb_index) {
    if (*bb == 0){
        *lsb_index = -1;
        return 0;
    }

    U64 lsb = *bb & -*bb;
    *lsb_index = __builtin_ctzll(lsb); // Get the index of the LSB
    *bb ^= lsb;
    return *bb;
}


// count the # of bits in an integer
// material evaluation and mobility
unsigned int cntbits(U64 bb){
    unsigned int count = 0;
    while(bb){
        bb &= (bb - 1);
        count++;
    }
    return count;
}

// get the details of a piece on square
int get_piece_on_square(Bitboards *bb, int square, int side) {
    U64 mask = 1ULL << square;
    if (bb->pawns[side] & mask) return (side == WHITE) ? wPawn : bPawn;
    if (bb->knights[side] & mask) return (side == WHITE) ? wKnight : bKnight;
    if (bb->bishops[side] & mask) return (side == WHITE) ? wBishop : bBishop;
    if (bb->rooks[side] & mask) return (side == WHITE) ? wRook : bRook;
    if (bb->queens[side] & mask) return (side == WHITE) ? wQueen : bQueen;
    if (bb->kings[side] & mask) return (side == WHITE) ? wKing : bKing;
    return EMPTY;
}

// get the bitboard array of piece on square
U64* get_piece_bitboard(Bitboards *bb, int piece) {
    switch (piece) {
        case wPawn: return &bb->pawns[WHITE];
        case bPawn: return &bb->pawns[BLACK];
        case wKnight: return &bb->knights[WHITE];
        case bKnight: return &bb->knights[BLACK];
        case wBishop: return &bb->bishops[WHITE];
        case bBishop: return &bb->bishops[BLACK];
        case wRook: return &bb->rooks[WHITE];
        case bRook: return &bb->rooks[BLACK];
        case wQueen: return &bb->queens[WHITE];
        case bQueen: return &bb->queens[BLACK];
        case wKing: return &bb->kings[WHITE];
        case bKing: return &bb->kings[BLACK];
        default: return NULL; // Should not happen
    }
}