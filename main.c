#include <stdio.h>
#include "bitboard.h"
#include "definitions.h"




// int main() {
//     // Initialize the bitboards to the starting position
//     Bitboards bitboards;
//     initialize_bitboards(&bitboards);

//     // Debug: Print initial white pawns
//     printf("Initial White Pawns:\n");
//     print_bitboard(bitboards.pawns[0]);

//     // Set a piece on e4 and clear the piece on e2
//     printf("Setting a piece on e4 and clearing the piece on e2:\n");

//     // Get the square indices for e4 and e2
//     int e4_square = algebraic_to_square("e4");
//     int e2_square = algebraic_to_square("e2");

//     // Set the bit for e4 and clear the bit for e2
//     SET_BIT(bitboards.pawns[0], e4_square); // Place a pawn on e4
//     CLEAR_BIT(bitboards.pawns[0], e2_square); // Remove the pawn from e2

//     printf("e4 -> %d\n", algebraic_to_square("e4")); // Expected: 28
//     printf("e2 -> %d\n", algebraic_to_square("e2")); // Expected: 12
//     printf("a8 -> %d\n", algebraic_to_square("a8")); // Expected: 0
//     printf("h1 -> %d\n", algebraic_to_square("h1")); // Expected: 63




//     // Debug: Print updated white pawns
//     printf("Updated White Pawns:\n");
//     print_bitboard(bitboards.pawns[0]);

//     // Debug: Print all pieces
//     printf("All Pieces:\n");
//     print_bitboard(bitboards.all_pieces);

//     return 0;
// }

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

    //set a bit
    printf("setting a piece e4:\n");
    int e4 = algebraic_to_square("e4");
    int e2 = algebraic_to_square("e2");

    // printf("Mask: %llu\n", mask);
    // U64 e2Mask = BIT(algebraic_to_square("e2"));
    SET_BIT(bitboards.pawns[0], e4);
    CLEAR_BIT(bitboards.pawns[0], e2);
    
    // Update the all_pieces bitboard
    bitboards.all_pieces = bitboards.pawns[0] | bitboards.pawns[1] |
                           bitboards.knights[0] | bitboards.knights[1] |
                           bitboards.bishops[0] | bitboards.bishops[1] |
                           bitboards.rooks[0] | bitboards.rooks[1] |
                           bitboards.queens[0] | bitboards.queens[1] |
                           bitboards.kings[0] | bitboards.kings[1];

    print_bitboard(bitboards.all_pieces);

    return 0;
}


