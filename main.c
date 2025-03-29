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

    // //set a bit
    // printf("setting a piece e4:\n");
    // // int e4 = algebraic_to_square("e4");
    // // int e2 = algebraic_to_square("e2");

    // // printf("Mask: %llu\n", mask);
    // // U64 e2Mask = BIT(algebraic_to_square("e2"));
    // setbit(bitboards.pawns[0], E4);
    // clearbit(bitboards.pawns[0], E2);
    
    // Update the all_pieces bitboard
    bitboards.all_pieces = bitboards.pawns[0] | bitboards.pawns[1] |
                           bitboards.knights[0] | bitboards.knights[1] |
                           bitboards.bishops[0] | bitboards.bishops[1] |
                           bitboards.rooks[0] | bitboards.rooks[1] |
                           bitboards.queens[0] | bitboards.queens[1] |
                           bitboards.kings[0] | bitboards.kings[1];

    print_bitboard(bitboards.all_pieces);

    U64 emptyboard = 0ULL;
    printf("empty board: \n");
    print_bitboard(emptyboard);

    printf("board after setting A8, E4, H1 \n");
    printf("value of A8: %d\n", A8); 
    printf("value of E4: %d\n", E4);
    printf("value of H1: %d\n", H1);
    setbit(emptyboard, A8);
    setbit(emptyboard, E4);
    setbit(emptyboard, H1);
    print_bitboard(emptyboard);

    int indices[64];
    int bitcounts;

    // Extract indices of set bits
    bitcounts = popbits(emptyboard, indices);

    // Print the results
    printf("Count: %d\n", bitcounts);
    printf("Indices of popped bits: ");
    for (int i = 0; i < bitcounts; i++) {
        char notation[3];
        square_to_algebraic(indices[i], notation),
        printf("%s:%d ", notation, indices[i]);
    }
    printf("\n");

    printf("empty board after popbits: \n");
    print_bitboard(emptyboard); 

    printf("Before popped, newboard:\n");
    U64 newboard = 0x0000000000000000;
    setbit(newboard, E2);
    setbit(newboard, E3);
    setbit(newboard, E4);

    print_bitboard(newboard);
    int indices1[64];

    int i;
    while (newboard){
        newboard = popabit(&newboard, &i);
        print_bitboard(newboard);
        printf("Index popped: %d\n", i);
    }

    printf("a8 index: %d\n", algebraic_to_square("a8")); // Expected: 0
    printf("h1 index: %d\n", algebraic_to_square("h1")); // Expected: 63
    printf("e4 index: %d\n", algebraic_to_square("e4")); // Expected: 36
    printf("e1 index: %d\n", algebraic_to_square("e1")); // Expected: 60

    setbit(newboard, E1);
    print_bitboard(newboard);
    printf("e1 index: %d\n", algebraic_to_square("e1")); // Expected: 60


    // Test the setbit first
    printf("Testing setbit function:\n");
    U64 testbb = 0;
    setbit(testbb, A8);  // Should be top-left
    setbit(testbb, H2);  // Should be top-left
    setbit(testbb, E4);  // Should be e4
    setbit(testbb, H1);  // Should be bottom-right
    print_bitboard(testbb);

    printf("Testing bitboard indices:\n");
    print_square_indices();

    



    return 0;
}


