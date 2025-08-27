#include "attack.h"
#include "movegen.h"
#include "definitions.h"

U64 get_rook_attacks(int sq, U64 blockers){
    U64 attacks = 0ULL;
    int directions[] = {NORTH, SOUTH, EAST, WEST};
    for (int i = 0; i < 4; i++) {
        int dir = directions[i];
        U64 attack_ray = sliding_rays[dir][sq];
        U64 first_blocker = attack_ray & blockers;
        if (first_blocker) {
            int blocker_sq = (dir == NORTH || dir == EAST) ? __builtin_ctzll(first_blocker) : (63 - __builtin_clzll(first_blocker));
            attacks |= attack_ray ^ sliding_rays[dir][blocker_sq];
        } else {
            attacks |= attack_ray;
        }
    }
    return attacks;
}

U64 get_bishop_attacks(int sq, U64 blockers) {
    U64 attacks = 0ULL;
    int directions[] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
    for (int i = 0; i < 4; i++) {
        int dir = directions[i];
        U64 attack_ray = sliding_rays[dir][sq];
        U64 first_blocker = attack_ray & blockers;
        if (first_blocker) {
            int blocker_sq = (dir == NORTH_EAST || dir == NORTH_WEST) ? __builtin_ctzll(first_blocker) : (63 - __builtin_clzll(first_blocker));
            attacks |= attack_ray ^ sliding_rays[dir][blocker_sq];
        } else {
            attacks |= attack_ray;
        }
    }
    return attacks;
}

int is_square_attacked(Bitboards *bb, int square, int attacking_side) {
    // The side being attacked is the opposite of the attacking_side
    int defending_side = 1 - attacking_side;

    // 1. Check for Pawn attacks
    // We check the pawn attack table for the *defending* side to see if
    // any of the attacker's pawns are on those squares.
    if (pawn_attacks[defending_side][square] & bb->pawns[attacking_side]) {
        return 1;
    }

    // 2. Check for Knight attacks
    if (knight_attack_table[square] & bb->knights[attacking_side]) {
        return 1;
    }

    // 3. Check for King attacks
    if (king_attack_table[square] & bb->kings[attacking_side]) {
        return 1;
    }

    // 4. Check for Bishop and Queen diagonal attacks
    U64 bishop_attacks = get_bishop_attacks(square, bb->all_pieces);
    if (bishop_attacks & (bb->bishops[attacking_side] | bb->queens[attacking_side])) {
        return 1;
    }

    // 5. Check for Rook and Queen straight attacks
    U64 rook_attacks = get_rook_attacks(square, bb->all_pieces);
    if (rook_attacks & (bb->rooks[attacking_side] | bb->queens[attacking_side])) {
        return 1;
    }

    // If no attacks are found, the square is safe
    return 0;
}
