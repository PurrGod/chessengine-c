#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include "definitions.h"

typedef struct {
    U64 pawns[2];    // 0 for white, 1 for black
    U64 knights[2];
    U64 bishops[2];
    U64 rooks[2];
    U64 queens[2];
    U64 kings[2];
    U64 occupied[2]; // 0 for white's occupied, 1 for black's occupied
    U64 all_pieces;  // All pieces combined (both white and black)
} Bitboards;

// Function prototypes
void initialize_bitboards(Bitboards *bb);
void print_bitboard(U64 bitboard);

void initialize_pawns(Bitboards *bb);
void initiaize_knights(Bitboards *bb);
void initialize_bishops(Bitboards *bb);
void initialize_rook(Bitboards *bb);
void initialize_queens(Bitboards *bb);
void initialize_kings(Bitboards *bb);


#endif
