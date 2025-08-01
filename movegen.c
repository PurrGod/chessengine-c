//movegen.c

#include "definitions.h"
#include "movegen.h"

/*

//rank maskings and file maskings
#define RANK_2_MASK 0x000000000000FF00ULL // White pawns' starting rank
#define RANK_7_MASK 0x00FF000000000000ULL // Black pawns' starting rank
#define FILE_A_MASK 0x0101010101010101ULL
#define FILE_H_MASK 0x8080808080808080ULL

*/

U64 knight_attack_table[64];
U64 king_attack_table[64];

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

U64 gen_pawn_moves(Bitboards *bb, int side) {
    U64 moves = 0ULL;
    U64 empty_squares = ~bb->all_pieces;

    U64 white_pawns = bb->pawns[WHITE];
    U64 black_pawns = bb->pawns[BLACK];

    if (side == WHITE) {
        // Single pushes
        U64 single_pushes = (white_pawns << 8) & empty_squares;
        moves |= single_pushes;

        // Double pushes
        U64 pawns_on_rank2 = white_pawns & RANK_2_MASK;
        U64 empty_rank3 = (pawns_on_rank2 << 8) & empty_squares;
        U64 double_pushes = (empty_rank3 << 8) & empty_squares;
        moves |= double_pushes;

        // Captures
        U64 NE_captures = ((white_pawns & ~FILE_H_MASK) << 9) & bb->occupied[BLACK];
        U64 NW_captures = ((white_pawns & ~FILE_A_MASK) << 7) & bb->occupied[BLACK];
        moves |= NE_captures | NW_captures;

    } else { // side == BLACK
        // Single pushes
        U64 single_pushes = (black_pawns >> 8) & empty_squares;
        moves |= single_pushes;

        // Double pushes
        U64 pawns_on_rank7 = black_pawns & RANK_7_MASK;
        U64 empty_rank6 = (pawns_on_rank7 >> 8) & empty_squares;
        U64 double_pushes = (empty_rank6 >> 8) & empty_squares;
        moves |= double_pushes;

        // Captures
        U64 SE_captures = ((black_pawns & ~FILE_A_MASK) >> 9) & bb->occupied[WHITE];
        U64 SW_captures = ((black_pawns & ~FILE_H_MASK) >> 7) & bb->occupied[WHITE];
        moves |= SE_captures | SW_captures;
    }

    return moves;
}
