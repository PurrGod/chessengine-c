//movegen.c

#include "definitions.h"
#include "movegen.h"
#include <stdio.h>

/*

//rank maskings and file maskings
#define RANK_2_MASK 0x000000000000FF00ULL // White pawns' starting rank
#define RANK_7_MASK 0x00FF000000000000ULL // Black pawns' starting rank
#define FILE_A_MASK 0x0101010101010101ULL
#define FILE_H_MASK 0x8080808080808080ULL

*/

U64 knight_attack_table[64];
U64 king_attack_table[64];
U64 sliding_rays[8][64];

static void add_move(moveList *list, int from, int to, int captured, int promotion, int flags) {
    // We will expand this later to handle flags
    list->moves[list->count] = (from) | (to << 6) | (captured << 12) | (promotion << 16);
    list->count++;
}

// helper functions for generating moves to append to move list
static void generate_pawn_move_list(Bitboards *bb, int side, moveList *list){
    U64 empty = ~bb->all_pieces;
    int opponent = 1 - side;

    // white pieces
    if (side == WHITE) {
        // pawn pushes
        U64 pawns = bb->pawns[WHITE];
        U64 single_pushes = (pawns << 8) & empty;
        U64 promotions = single_pushes & RANK_8_MASK;
        single_pushes &= ~RANK_8_MASK;
        U64 double_pushes = ((pawns & RANK_2_MASK) << 16) & empty & (empty << 8);
        while (single_pushes) {int to_sq; popabit(&single_pushes, &to_sq); add_move(list, to_sq - 8, to_sq, EMPTY, EMPTY, 0);}
        while (double_pushes) {int to_sq; popabit(&double_pushes, &to_sq); add_move(list, to_sq - 16, to_sq, EMPTY, EMPTY, 0);}
        while (promotions) {
            int to_sq;
            popabit(&promotions, &to_sq);
            add_move(list, to_sq - 8, to_sq, EMPTY, wQueen, 0);
        }

        // pawn captures
        U64 nw_cap = ((pawns & ~FILE_A_MASK) << 7) & bb->occupied[opponent];
        U64 ne_cap = ((pawns & ~FILE_H_MASK) << 9) & bb->occupied[opponent];

        U64 nw_cap_promo = nw_cap & RANK_8_MASK;
        nw_cap &= ~RANK_8_MASK;
        U64 ne_cap_promo = ne_cap & RANK_8_MASK;
        ne_cap &= ~RANK_8_MASK;

        while(nw_cap) {int to; popabit(&nw_cap, &to); add_move(list, to - 7, to, bPawn, EMPTY, 0);} 
        while(ne_cap) {int to; popabit(&ne_cap, &to); add_move(list, to - 9, to, bPawn, EMPTY, 0);} 

        // STILL NEEDS WORK HERE //
    } else {
        U64 pawns = bb->pawns[BLACK];
        U64 single_pushes = (pawns >> 8) & empty;
        U64 promotions = single_pushes & RANK_1_MASK;
        single_pushes &= ~RANK_1_MASK;
        U64 double_pushes = ((pawns & RANK_7_MASK) >> 16) & empty & (empty >> 8);

        while (single_pushes) {int to_sq; popabit(&single_pushes, &to_sq); add_move(list, to_sq + 8, to_sq, EMPTY, EMPTY, 0);}
        while (double_pushes) {int to_sq; popabit(&double_pushes, &to_sq); add_move(list, to_sq + 16, to_sq, EMPTY, EMPTY, 0);}
        while (promotions) {
            int to_sq;
            popabit(promotions, to_sq);
            add_move(list, to_sq + 8, to_sq, EMPTY, bQueen, 0);
        }
    }
}

void generate_all_moves(Bitboards *bb, moveList *list, int side) {
    if (list == NULL){
        return;
    }
    list->count = 0;

    generate_pawn_move_list(bb, side, list);

}

void init_knight_attacks(){
    // empty bitboard for knight attacks
    for (int square = 0; square < 64; square++){
        knight_attack_table[square] = 0ULL; //empty board for that particular square

        int rank = square / 8;
        int file = square % 8;

        // knight attack directions
        // {2,1} means knight attacks 2 ranks up and 1 file right
        int knight_moves[8][2] = {
            // first column represents change in rank
            // second column represens change in file
            {  2  ,   1}, 
            {  2  ,  -1}, 
            { -2  ,   1}, 
            { -2  ,  -1},
            {  1  ,   2}, 
            {  1  ,  -2}, 
            { -1  ,   2}, 
            { -1  ,  -2}
        };

        // set the bits for each attack for the knight on the attack table
        for (int i = 0; i < 8; i++){
            int new_rank = rank + knight_moves[i][0]; // first col in knight_moves
            int new_file = file + knight_moves[i][1]; // second col in knight_moves

            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8){
                setbit(knight_attack_table[square], (new_rank * 8 + new_file));
            }
        }
    }

}

void init_king_attacks(){
    // empty bitboard for king attacks
    for (int square = 0; square < 64; square++){
        king_attack_table[square] = 0ULL; //empty board for that particular square

        int rank = square / 8;
        int file = square % 8;

        // king attack directions
        int king_moves[8][2] = {
            { 1, 0},   // North
            { -1, 0},  // South
            { 0, 1},   // East
            { 0, -1},  // West
            { 1, 1},   // North-East
            { -1, -1}, // South-West
            { -1, 1},  // North-West
            { 1, -1}   // South-East
        };

        // set the bits for each attack for the king on the attack table
        for (int i = 0; i < 8; i++){
            int new_rank = rank + king_moves[i][0]; // first col in king_moves
            int new_file = file + king_moves[i][1]; // second col in king_moves

            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8){
                setbit(king_attack_table[square], (new_rank * 8 + new_file));
            }
        }
    }
}   



// generate bitboards for every direction
void init_sliding_rays(){

    for (int sq = 0; sq < 64; sq++){
        int rank_start = sq / 8;
        int file_start = sq % 8;

        // clear previous table
        for (int dir = 0; dir < 8; dir++){
            sliding_rays[dir][sq] = 0ULL;
        }

        // North
        for (int r = rank_start + 1; r < 8; r++) setbit(sliding_rays[NORTH][sq], r * 8 + file_start);
        // SOUTH
        for (int r = rank_start - 1; r >= 0; r--) setbit(sliding_rays[SOUTH][sq], r * 8 + file_start);
        // EAST
        for (int f = file_start + 1; f < 8; f++) setbit(sliding_rays[EAST][sq], rank_start * 8 + f);
        // WEST
        for (int f = file_start - 1; f >= 0; f--) setbit(sliding_rays[WEST][sq], rank_start * 8 + f);

        // NE
        for (int r = rank_start + 1, f = file_start + 1; r < 8 && f < 8; r++, f++) setbit(sliding_rays[NORTH_EAST][sq], r * 8 + f);
        // SE
        for (int r = rank_start - 1, f = file_start + 1; r >= 0 && f < 8; r--, f++) setbit(sliding_rays[SOUTH_EAST][sq], r * 8 + f);
        // SW
        for (int r = rank_start - 1, f = file_start - 1; r >= 0 && f >= 0; r--, f--) setbit(sliding_rays[SOUTH_WEST][sq], r * 8 + f);
        // NW
        for (int r = rank_start + 1, f = file_start - 1; r < 8 && f >= 0; r++, f--) setbit(sliding_rays[NORTH_WEST][sq], r * 8 + f);
    }
}

// generate rook moves
// pseudocode for generating rook moves
/*

*/
// U64 gen_pawn_moves(Bitboards *bb, int side) {
//     U64 moves = 0ULL;
//     U64 empty_squares = ~bb->all_pieces;

//     U64 white_pawns = bb->pawns[WHITE];
//     U64 black_pawns = bb->pawns[BLACK];

//     if (side == WHITE) {
//         // Single pushes
//         U64 single_pushes = (white_pawns << 8) & empty_squares;
//         moves |= single_pushes;

//         // Double pushes
//         U64 pawns_on_rank2 = white_pawns & RANK_2_MASK;
//         U64 empty_rank3 = (pawns_on_rank2 << 8) & empty_squares;
//         U64 double_pushes = (empty_rank3 << 8) & empty_squares;
//         moves |= double_pushes;

//         // Captures
//         U64 NE_captures = ((white_pawns & ~FILE_H_MASK) << 9) & bb->occupied[BLACK];
//         U64 NW_captures = ((white_pawns & ~FILE_A_MASK) << 7) & bb->occupied[BLACK];
//         moves |= NE_captures | NW_captures;

//     } else { // side == BLACK
//         // Single pushes
//         U64 single_pushes = (black_pawns >> 8) & empty_squares;
//         moves |= single_pushes;

//         // Double pushes
//         U64 pawns_on_rank7 = black_pawns & RANK_7_MASK;
//         U64 empty_rank6 = (pawns_on_rank7 >> 8) & empty_squares;
//         U64 double_pushes = (empty_rank6 >> 8) & empty_squares;
//         moves |= double_pushes;

//         // Captures
//         U64 SE_captures = ((black_pawns & ~FILE_A_MASK) >> 9) & bb->occupied[WHITE];
//         U64 SW_captures = ((black_pawns & ~FILE_H_MASK) >> 7) & bb->occupied[WHITE];
//         moves |= SE_captures | SW_captures;
//     }

//     return moves;
// }

// U64 gen_rook_moves(Bitboards *bb, int side) {
//     U64 moves = 0ULL;
//     U64 occupied = bb->all_pieces;

//     U64 rooks = bb->rooks[side];

//     // loop through each of the rooks
//     while (rooks){
//         int start_square;

//         popabit(&rooks, &start_square);

//         // North
//         U64 north_ray = sliding_rays[NORTH][start_square];
//         U64 north_blockers = north_ray & occupied;
//         if (north_blockers) {
//             int first_blocker = __builtin_ctzll(north_blockers);
//             north_ray ^= sliding_rays[NORTH][first_blocker];
//         }

//         moves |= north_ray;

//         // South
//         U64 south_ray = sliding_rays[SOUTH][start_square];
//         U64 south_blockers = south_ray & occupied;
//         if (south_blockers) {
//             int first_blocker = __builtin_clzll(south_blockers);
//             south_ray ^= sliding_rays[SOUTH][63 - first_blocker];
//         }

//         moves |= south_ray;

//         // East
//         U64 east_ray = sliding_rays[EAST][start_square];
//         U64 east_blockers = east_ray & occupied;
//         if (east_blockers) {
//             int first_blocker = __builtin_ctzll(east_blockers);
//             east_ray ^= sliding_rays[EAST][first_blocker];
//         }

//         moves |= east_ray;

//         // West
//         U64 west_ray = sliding_rays[WEST][start_square];
//         U64 west_blockers = west_ray & occupied;
//         if (west_blockers) {
//             int first_blocker = __builtin_clzll(west_blockers);
//             west_ray ^= sliding_rays[WEST][63 - first_blocker];
//         }

//         moves |= west_ray;

//     }

//     // removes moves that landed on our own pieces
//     return moves & ~bb->occupied[side]; 

// }


// U64 gen_bishop_moves(Bitboards *bb, int side) {
//     U64 moves = 0ULL;
//     U64 occupied = bb->all_pieces;

//     U64 bishops = bb->bishops[side];

//     // loop through each of the bishops
//     while (bishops){
//         int start_square;

//         popabit(&bishops, &start_square);

//         // North-East - (+)
//         U64 ne_ray = sliding_rays[NORTH_EAST][start_square];
//         U64 ne_blockers = ne_ray & occupied;
//         if (ne_blockers) {
//             int first_blocker = __builtin_ctzll(ne_blockers);
//             ne_ray ^= sliding_rays[NORTH_EAST][first_blocker];
//         }

//         moves |= ne_ray;

//         // South-East - (-)
//         U64 se_ray = sliding_rays[SOUTH_EAST][start_square];
//         U64 se_blockers = se_ray & occupied;
//         if (se_blockers) {
//             int first_blocker = __builtin_clzll(se_blockers);
//             se_ray ^= sliding_rays[SOUTH_EAST][63 - first_blocker];
//         }

//         moves |= se_ray;

//         // South-West - (-)
//         U64 sw_ray = sliding_rays[SOUTH_WEST][start_square];
//         U64 sw_blockers = sw_ray & occupied;
//         if (sw_blockers) {
//             int first_blocker = __builtin_clzll(sw_blockers);
//             sw_ray ^= sliding_rays[SOUTH_WEST][63 - first_blocker];
//         }

//         moves |= sw_ray;

//         // North-West - (+)
//         U64 nw_ray = sliding_rays[NORTH_WEST][start_square];
//         U64 nw_blockers = nw_ray & occupied;
//         if (nw_blockers) {
//             int first_blocker = __builtin_ctzll(nw_blockers);
//             nw_ray ^= sliding_rays[NORTH_WEST][first_blocker];
//         }

//         moves |= nw_ray;

//     }

//     // removes moves that landed on our own pieces
//     return moves & ~bb->occupied[side]; 
// }


// U64 gen_queen_moves(Bitboards *bb, int side) {
//     // Queen moves are a combination of rook and bishop moves
//     U64 original_rook = bb->rooks[side];
//     U64 original_bishop = bb->bishops[side];

//     bb->rooks[side] = bb->queens[side];
//     bb->bishops[side] = bb->queens[side];

//     U64 rook_moves = gen_rook_moves(bb, side);
//     U64 bishop_moves = gen_bishop_moves(bb, side);

//     bb->rooks[side] = original_rook;
//     bb->bishops[side] = original_bishop;
    
//     return rook_moves | bishop_moves;
// }


// U64 gen_king_moves(Bitboards *bb, int side) {
//     U64 moves = 0ULL;
//     U64 occupied = bb->all_pieces;

//     U64 kings = bb->kings[side];

//     // loop through each of the kings
//     while (kings){
//         int start_square;

//         popabit(&kings, &start_square);

//         // King moves are simply the attack table for the king
//         moves |= king_attack_table[start_square];
//     }

//     // removes moves that landed on our own pieces
//     return moves & ~bb->occupied[side]; 
// }


// U64 gen_knight_moves(Bitboards *bb, int side) {
    U64 moves = 0ULL;
    U64 occupied = bb->all_pieces;

    U64 knights = bb->knights[side];

    // loop through each of the knights
    while (knights){
        int start_square;

        popabit(&knights, &start_square);

        // Knight moves are simply the attack table for the knight
        moves |= knight_attack_table[start_square];
    }

    // removes moves that landed on our own pieces
    return moves & ~bb->occupied[side]; 
}