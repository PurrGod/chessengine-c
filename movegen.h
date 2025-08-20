//movegen.h

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "definitions.h"

// tables
extern U64 knight_attack_table[64];
extern U64 king_attack_table[64];
extern U64 sliding_rays[8][64];
extern U64 pawn_attacks[2][64];

// initializations
void init_knight_attacks();
void init_king_attacks();
void init_sliding_rays();
void init_pawn_attacks();
// These masks are used to determine the squares attacked by knights

// These functions generate pseudo-legal moves for each piece type
void generate_all_moves(Bitboards *bb, int side, moveList *list);


#endif // MOVEGEN_H