#ifndef BITBOARD_H
#define BITBOARD_H

#include "definitions.h"

// Function prototypes
void initialize_bitboards(Bitboards *bb);

// Debug function to print a bitboard
void print_bitboard(U64 bb);
// void iwroteprintbitboard(U64 bb);
void print_square_indices();

void print_board(const Bitboards *bb);
void parse_fen(Bitboards *bb, const char *fen);

// these followiing functions are to initialize the bitboards
void initialize_pawns(Bitboards *bb);
void initialize_knights(Bitboards *bb);
void initialize_bishops(Bitboards *bb);
void initialize_rook(Bitboards *bb);
void initialize_queens(Bitboards *bb);
void initialize_kings(Bitboards *bb);
void board_to_fen(Bitboards *bb, char *fen_str);

#endif
