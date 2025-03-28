#ifndef BITBOARD_H
#define BITBOARD_H

#include "definitions.h"

// Function prototypes
void initialize_bitboards(Bitboards *bb);

// Debug function to print a bitboard
void print_bitboard(U64 bitboard);

// these followiing functions are to initialize the bitboards
void initialize_pawns(Bitboards *bb);
void initiaize_knights(Bitboards *bb);
void initialize_bishops(Bitboards *bb);
void initialize_rook(Bitboards *bb);
void initialize_queens(Bitboards *bb);
void initialize_kings(Bitboards *bb);


#endif
