#include "definitions.h"
#include "movegen.h"
#include "bitboard.h"
#include <stdio.h>
#include <stdlib.h>

// main function to test move generation
int main() {
    Bitboards bb;
    initialize_bitboards(&bb);
    printf("Initial Bitboards:\n");
    print_bitboard(bb.all_pieces);
}