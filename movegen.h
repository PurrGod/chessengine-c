//movegen.h

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "definitions.h"

// Rank masks for move generation

// attack masks for pawns
// These masks are used to determine the squares attacked by pawns
U64 pawn_attackMasks(U64 bb, int side);

// 
extern U64 knight_attack_table[64];
extern U64 king_attack_table[64];

void init_knight_attacks();
void init_king_attacks();
void init_sliding_rays();
// These masks are used to determine the squares attacked by knights

// generating pawn moves
// creates pseudo-legal moves for pawns (doesn't check for legality or king checks)
// not an init function but generates moves
U64 gen_pawn_moves(Bitboards *bb, int side);
U64 gen_rook_moves(Bitboards *bb, int side);
U64 gen_bishop_moves(Bitboards *bb, int side);
U64 gen_queen_moves(Bitboards *bb, int side);
U64 gen_king_moves(Bitboards *bb, int side);
U64 gen_knight_moves(Bitboards *bb, int side);
// These functions generate pseudo-legal moves for each piece type

// generating moves to add to movelist
// this function, encodes the moves to 32 bit integer
static void add_move(moveList *list, int sq_from, int sq_to, int captured, int promotion, int flags);
static void generate_pawn_move_list(Bitboards *bb, int side, moveList *list);

void generate_all_moves(Bitboards *bb, moveList *list, int side);






#endif // MOVEGEN_H