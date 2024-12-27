#include <stdio.h>
#include "bitboard.h"
#include "definitions.h"

int main() {
    // Create a bitboard struct
    Bitboards bitboards;

    // Initialize the bitboards to the starting position
    initialize_bitboards(&bitboards);

    // Debug: Print white pawns
    printf("White Pawns:\n");
    print_bitboard(bitboards.pawns[0]);

    // Debug: Print black pawns
    printf("Black Pawns:\n");
    print_bitboard(bitboards.pawns[1]);

    // Debug: print queens
    printf("all queens:\n");
    print_bitboard(bitboards.queens[1] | bitboards.queens[0]);

    // knights
    printf("knights:\n");
    print_bitboard(bitboards.knights[0] | bitboards.knights[1]);

    // Debug: Print all occupied squares
    printf("All Pieces:\n");
    print_bitboard(bitboards.all_pieces);

    return 0;
}


