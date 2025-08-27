#include "definitions.h"

U64 get_bishop_attacks(int sq, U64 blockers);
U64 get_rook_attacks(int sq, U64 blockers);


int is_square_attacked(Bitboards *bb, int square, int attacking_side);