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

static const int is_positive_direction[8] = {
    1, // NORTH
    1, // EAST
    0, // SOUTH
    0, // WEST
    1, // NORTH_EAST
    0, // SOUTH_EAST
    0, // SOUTH_WEST
    1  // NORTH_WEST
};

U64 knight_attack_table[64];
U64 king_attack_table[64];
U64 sliding_rays[8][64];
U64 pawn_attacks[2][64];


static void add_move(moveList *list, int from, int to, int captured, int promotion, int flags) {
    // We will expand this later to handle flags
    list->moves[list->count] = (to) | (from << 6) | (captured << 12) | (promotion << 16) | (flags);
    list->count++;
}

static int get_piece_on_square(Bitboards *bb, int square, int side) {
    U64 mask = 1ULL << square;
    if (bb->pawns[side] & mask) return (side == WHITE) ? wPawn : bPawn;
    if (bb->knights[side] & mask) return (side == WHITE) ? wKnight : bKnight;
    if (bb->bishops[side] & mask) return (side == WHITE) ? wBishop : bBishop;
    if (bb->rooks[side] & mask) return (side == WHITE) ? wRook : bRook;
    if (bb->queens[side] & mask) return (side == WHITE) ? wQueen : bQueen;
    if (bb->kings[side] & mask) return (side == WHITE) ? wKing : bKing;
    return EMPTY;
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
            add_move(list, to_sq - 8, to_sq, EMPTY, wRook, 0);
            add_move(list, to_sq - 8, to_sq, EMPTY, wBishop, 0);
            add_move(list, to_sq - 8, to_sq, EMPTY, wKnight, 0);
            
        }

        // pawn captures
        U64 nw_cap = ((pawns & ~FILE_A_MASK) << 7) & bb->occupied[opponent];
        U64 ne_cap = ((pawns & ~FILE_H_MASK) << 9) & bb->occupied[opponent];

        U64 nw_cap_promo = nw_cap & RANK_8_MASK;
        nw_cap &= ~RANK_8_MASK;
        U64 ne_cap_promo = ne_cap & RANK_8_MASK;
        ne_cap &= ~RANK_8_MASK;

        while(nw_cap) {int to; popabit(&nw_cap, &to); int captured = get_piece_on_square(bb, to, opponent); add_move(list, to - 7, to, captured, EMPTY, 0);} 
        while(ne_cap) {int to; popabit(&ne_cap, &to); int captured = get_piece_on_square(bb, to, opponent); add_move(list, to - 9, to, captured, EMPTY, 0);} 

        // promotion capture moves
        while(nw_cap_promo) {
            int to_sq;
            popabit(&nw_cap_promo, &to_sq); int captured = get_piece_on_square(bb, to_sq, opponent);
            add_move(list, to_sq - 7, to_sq, captured, wQueen, 0);
            add_move(list, to_sq - 7, to_sq, captured, wRook, 0);
            add_move(list, to_sq - 7, to_sq, captured, wBishop, 0);
            add_move(list, to_sq - 7, to_sq, captured, wKnight, 0);
        }

        while(ne_cap_promo) {
            int to_sq;
            popabit(&ne_cap_promo, &to_sq); int captured = get_piece_on_square(bb, to_sq, opponent);
            add_move(list, to_sq - 9, to_sq, captured, wQueen, 0);
            add_move(list, to_sq - 9, to_sq, captured, wRook, 0);
            add_move(list, to_sq - 9, to_sq, captured, wBishop, 0);
            add_move(list, to_sq - 9, to_sq, captured, wKnight, 0);
        }

        // En Passant
        // Logic
        /*
        We make use of the "En Pas" variable in the bb structure. En Pas is given a potential capture
        square value ONLY when the opponent made a double pawn push prior. For example, if black pawn moved 
        from d7 to d5 (double push) then for a potential en passant, white can captue on d6.

        We need to find out what white pawns can do this only if the en pas value is True.
        - Since a white pawn can capture on en passant, the square it captures and moves to is essentially
          the single black pawn push so in this case it would be d6. 
        
          We can use this information to isolate white pawns that can caputure such as pawns on e5 and c5 which can 
          capture on d6.

          First we take the attack table of a pawn on d6 which would give us the bit mask of e5 and c5. We can then
          do an and operation of this mask with the white pawns to isolate whatever pawns on e5 and c5 for example.
          We can then add this to a captured bitboard and then pop each square and append to move list.
        */

        if (bb->enPas != NO_SQ) {
            // find captures of black pawn on single push
            U64 enpass_captures = pawn_attacks[opponent][bb->enPas];
            enpass_captures &= pawns;

            while(enpass_captures) {
                int to_sq = bb->enPas;
                int from;
                popabit(&enpass_captures, &from);
                add_move(list, from, to_sq, bPawn, EMPTY, MOVE_IS_ENPASSANT);
            }
        }

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
            popabit(&promotions, &to_sq);
            add_move(list, to_sq + 8, to_sq, EMPTY, bQueen, 0);
        }

        // pawn captures
        U64 sw_cap = ((pawns & ~FILE_A_MASK) >> 9) & bb->occupied[opponent];
        U64 se_cap = ((pawns & ~FILE_H_MASK) >> 7) & bb->occupied[opponent];

        U64 sw_cap_promo = sw_cap & RANK_1_MASK;
        sw_cap &= ~RANK_1_MASK;
        U64 se_cap_promo = se_cap & RANK_1_MASK;
        se_cap &= ~RANK_1_MASK;

        while(sw_cap) { int to; popabit(&sw_cap, &to); int cap = get_piece_on_square(bb, to, opponent); add_move(list, to + 9, to, cap, EMPTY, 0); }
        while(se_cap) { int to; popabit(&se_cap, &to); int cap = get_piece_on_square(bb, to, opponent); add_move(list, to + 7, to, cap, EMPTY, 0); }

        while(sw_cap_promo) {
            int to; popabit(&sw_cap_promo, &to); int cap = get_piece_on_square(bb, to, opponent);
            add_move(list, to + 9, to, cap, bQueen, 0); add_move(list, to + 9, to, cap, bRook, 0);
            add_move(list, to + 9, to, cap, bBishop, 0); add_move(list, to + 9, to, cap, bKnight, 0);
        }
        while(se_cap_promo) {
            int to; popabit(&se_cap_promo, &to); int cap = get_piece_on_square(bb, to, opponent);
            add_move(list, to + 7, to, cap, bQueen, 0); add_move(list, to + 7, to, cap, bRook, 0);
            add_move(list, to + 7, to, cap, bBishop, 0); add_move(list, to + 7, to, cap, bKnight, 0);
        }

        // En Passant
        if (bb->enPas != NO_SQ) {
            U64 en_attackers = pawn_attacks[opponent][bb->enPas] & pawns;
            while (en_attackers) {
                int from;
                int to_sq = bb->enPas;
                popabit(&en_attackers, &from);
                add_move(list, from, to_sq, wPawn, EMPTY, MOVE_IS_ENPASSANT);
            }
        }

    }
}

// So what do we do here?
// The goal is to append pseudolegal moves into the moveList.
// How do we do it with knights?
// we already have the attack tables of the knights
// we can start with white

// define knights based on the side, then we calculate legal moves where squares are empty
// simple and operations with ~occupied can do the job and then we can use the 
    // popabit and add_move function effectively using a while loop.
    // these can also be known as silent moves

// Next we need to do look at captures.
// I feel like the knight function is a lot simpler than the pawn function.
static void generate_knight_move_list(Bitboards *bb, int side, moveList *list) {
    int opponent = 1 - side;
    U64 knights = bb->knights[side];
    U64 empty = ~bb->all_pieces;
    U64 enemy_pieces = bb->occupied[opponent];

    // silent moves (no captures)
    while (knights) {
        // from square
        int from;
        popabit(&knights, &from);
        
        U64 attacks = knight_attack_table[from];
        U64 silent_leaps = attacks & empty; // silent leaps will only contain squares without pieces, our piece or not
        U64 captures = attacks & enemy_pieces;

        // process silent moves
        while (silent_leaps) {
            int to_sq; popabit(&silent_leaps, &to_sq);
            add_move(list, from, to_sq, EMPTY, EMPTY, 0);
        }

        // process captures
        while (captures) {
            int cap_sq; popabit(&captures, &cap_sq);
            add_move(list, from, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
        }
    }
}


// We implement the kindergarten bitboard logic
// Essentially we make use of the ray array that we have initialized in the beginning
// sliding_rays[8][64] 
static void generate_rook_move_list(Bitboards *bb, int side, moveList *list) {
    int opponent = 1 - side;
    U64 rooks = bb->rooks[side];
    U64 empty = ~bb->all_pieces;
    U64 enemy_pcs = bb->occupied[opponent];

    // directions for Rooks:
    int rook_directions[] = {NORTH, EAST, SOUTH, WEST};

    // iterate till every rooked is accounted for and popped
    while (rooks) {
        // keep track of from square
        int from_sq;
        popabit(&rooks, &from_sq);

        // iterate through the directions

        for (int i = 0; i < 4; i++){
            int curr_direction = rook_directions[i];
            U64 attack_ray = sliding_rays[curr_direction][from_sq];
            U64 attack_squares;

            U64 attack_blockers = attack_ray & bb->all_pieces; // finds blockers for that specific array
            if (attack_blockers) {
                int first_blocker;
                if (i<2){ // North and East
                    first_blocker = __builtin_ctzll(attack_blockers);
                } else {first_blocker =  (63 - (__builtin_clzll(attack_blockers)));}
                attack_squares = attack_ray ^ sliding_rays[curr_direction][first_blocker];
            } else {attack_squares = attack_ray;}

            // divide captures and quiet moves
            U64 captures = attack_squares & enemy_pcs;
            U64 silent_moves = attack_squares & empty;

            // time to add these moves to add list
            while (captures){
                int cap_sq;
                popabit(&captures, &cap_sq);
                add_move(list, from_sq, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
            }

            while (silent_moves) {
                int to_sq;
                popabit(&silent_moves, &to_sq);
                add_move(list, from_sq, to_sq, EMPTY, EMPTY, 0);
            }
        }
    }
}


static void generate_bishop_move_list(Bitboards *bb, int side, moveList *list) {
    int opponent = 1 - side;
    U64 bishops = bb->bishops[side];
    U64 empty = ~bb->all_pieces;
    U64 enemy = bb->occupied[opponent];

    // directions of bishop from positive to negative directions
    int bishop_directions[] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
    //                         ^4          ^7          ^5          ^6

    while (bishops) {
        int from_sq;
        popabit(&bishops, &from_sq);

        // iterate through the directions
        for (int i = 0; i < 4; i++) {
            int curr_direction = bishop_directions[i];
            U64 attack_ray = sliding_rays[curr_direction][from_sq];
            U64 attack_squares;

            U64 attack_blockers = attack_ray & bb->all_pieces;
            if (attack_blockers){
                int first_blocker;
                if (is_positive_direction[curr_direction]){
                    first_blocker = __builtin_ctzll(attack_blockers);
                } else{first_blocker = (63 - (__builtin_clzll(attack_blockers)));}
                attack_squares = attack_ray ^ sliding_rays[curr_direction][first_blocker];
            } else {attack_squares = attack_ray;}

            U64 captures = attack_squares & enemy;
            U64 silent_moves = attack_squares & empty;

            while (captures) {
                int cap_sq;
                popabit(&captures, &cap_sq);
                add_move(list, from_sq, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
            }

            while (silent_moves) {
                int to_sq;
                popabit(&silent_moves, &to_sq);
                add_move(list, from_sq, to_sq, EMPTY, EMPTY, 0);
            }
        }
    }

}

// Queen move gen, copy paste bishop and rook move gen code
static void generate_queen_move_list(Bitboards *bb, int side, moveList *list) {
    U64 queens = bb->queens[side];
    int opponent = 1 - side;

    int straight_directions[] = {NORTH, EAST, SOUTH, WEST};
    int diagonal_directions[] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

    while (queens) {
        int from_sq;
        popabit(&queens, &from_sq);
        
        // encode moves for up down left and right
        for (int i = 0; i < 4; i++){
            int curr_direction = straight_directions[i];
            U64 attack_ray = sliding_rays[curr_direction][from_sq];
            U64 attack_squares;

            U64 attack_blockers = attack_ray & bb->all_pieces; // finds blockers for that specific array
            if (attack_blockers) {
                int first_blocker;

                if (is_positive_direction[curr_direction]){ // North and East
                    first_blocker = __builtin_ctzll(attack_blockers);
                } else {first_blocker =  (63 - (__builtin_clzll(attack_blockers)));}
                attack_squares = attack_ray ^ sliding_rays[curr_direction][first_blocker];
            } else {attack_squares = attack_ray;}

            // divide captures and quiet moves
            U64 captures = attack_squares & bb->occupied[opponent];
            U64 silent_moves = attack_squares & ~bb->all_pieces;

            // time to add these moves to add list
            while (captures){
                int cap_sq;
                popabit(&captures, &cap_sq);
                add_move(list, from_sq, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
            }

            while (silent_moves) {
                int to_sq;
                popabit(&silent_moves, &to_sq);
                add_move(list, from_sq, to_sq, EMPTY, EMPTY, 0);
            }
        }

        // diagonal moves
        for (int i = 0; i < 4; i++) {
            int curr_direction = diagonal_directions[i];
            U64 attack_ray = sliding_rays[curr_direction][from_sq];
            U64 attack_squares;

            U64 attack_blockers = attack_ray & bb->all_pieces;
            if (attack_blockers){
                int first_blocker;
                if (is_positive_direction[curr_direction]){
                    first_blocker = __builtin_ctzll(attack_blockers);
                } else{first_blocker = (63 - (__builtin_clzll(attack_blockers)));}
                attack_squares = attack_ray ^ sliding_rays[curr_direction][first_blocker];
            } else {attack_squares = attack_ray;}

            U64 captures = attack_squares & bb->occupied[opponent];
            U64 silent_moves = attack_squares & ~bb->all_pieces;

            while (captures) {
                int cap_sq;
                popabit(&captures, &cap_sq);
                add_move(list, from_sq, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
            }

            while (silent_moves) {
                int to_sq;
                popabit(&silent_moves, &to_sq);
                add_move(list, from_sq, to_sq, EMPTY, EMPTY, 0);
            }
        }

    }
}

// king move gen, pretty simple just check for captures and silent moves
static void generate_king_move_list(Bitboards *bb, int side, moveList *list) {
    if (bb->kings[side] == 0) {
        return;
    }
    
    int opponent = 1 - side;
    U64 king = bb->kings[side];

    int from_sq = __builtin_ctzll(king);
    U64 king_attacks = king_attack_table[from_sq];

    U64 valid_moves = king_attacks & ~bb->occupied[side];

    // captures
    U64 attack_squares = valid_moves & bb->occupied[opponent];
    U64 silent_moves = valid_moves & ~bb->all_pieces;

    while (attack_squares){
        int cap_sq;
        popabit(&attack_squares, &cap_sq);
        add_move(list, from_sq, cap_sq, get_piece_on_square(bb, cap_sq, opponent), EMPTY, 0);
    }

    while (silent_moves){
        int to_sq;
        popabit(&silent_moves, &to_sq);
        add_move(list, from_sq, to_sq, EMPTY, EMPTY, 0);
    }
}


//fills in the attack tables for quick look ups
void init_all_piece_tables() {
    init_pawn_attacks();
    init_knight_attacks();
    init_king_attacks();
    init_sliding_rays();
}

// initialize pawn attack tables
void init_pawn_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        pawn_attacks[WHITE][sq] = 0ULL;
        pawn_attacks[BLACK][sq] = 0ULL;
        if ((1ULL << sq) & ~FILE_A_MASK) setbit(pawn_attacks[WHITE][sq], sq + 7);
        if ((1ULL << sq) & ~FILE_H_MASK) setbit(pawn_attacks[WHITE][sq], sq + 9);
        if ((1ULL << sq) & ~FILE_H_MASK) setbit(pawn_attacks[BLACK][sq], sq - 7);
        if ((1ULL << sq) & ~FILE_A_MASK) setbit(pawn_attacks[BLACK][sq], sq - 9);
    }  
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

void generate_all_moves(Bitboards *bb, int side, moveList *list) {
    if (list == NULL){
        return;
    }
    list->count = 0;

    generate_pawn_move_list(bb, side, list);
    generate_knight_move_list(bb, side, list);
    generate_rook_move_list(bb, side, list);
    generate_bishop_move_list(bb, side, list);
    generate_queen_move_list(bb, side, list);
    generate_king_move_list(bb, side, list);

}
