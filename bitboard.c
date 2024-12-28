#include <stdio.h>
#include "bitboard.h"

// Initialize bitboards to default starting position
void initialize_bitboards(Bitboards *bb) {
    initialize_pawns(bb);
    initiaize_knights(bb);
    initialize_bishops(bb);
    initialize_rook(bb);
    initialize_queens(bb);
    initialize_kings(bb);

    // Occupied squares
    bb->occupied[0] = bb->pawns[0] | bb->knights[0] | bb->bishops[0] | bb->rooks[0] | bb->queens[0] | bb->kings[0];
    bb->occupied[1] = bb->pawns[1] | bb->knights[1] | bb->bishops[1] | bb->rooks[1] | bb->queens[1] | bb->kings[1];

    // All pieces
    bb->all_pieces = bb->occupied[0] | bb->occupied[1];
}

void initialize_pawns(Bitboards *bb){
    // Pawns
    bb->pawns[0] = 0x000000000000FF00;  // White pawns
    bb->pawns[1] = 0x00FF000000000000;  // Black pawns
}

void initiaize_knights(Bitboards *bb){
    // Knights
    bb->knights[0] = 0x0000000000000042; // White knights (b1, g1)
    bb->knights[1] = 0x4200000000000000; // Black knights (b8, g8)
}

void initialize_bishops(Bitboards *bb){
    // Bishops
    bb->bishops[0] = 0x0000000000000024; // White bishops (c1, f1)
    bb->bishops[1] = 0x2400000000000000; // Black bishops (c8, f8)
}

void initialize_rook(Bitboards *bb){
    // Rooks
    bb->rooks[0] = 0x0000000000000081;   // White rooks (a1, h1)
    bb->rooks[1] = 0x8100000000000000;   // Black rooks (a8, h8)
}

void initialize_queens(Bitboards *bb){
    // Queens
    bb->queens[0] = 0x0000000000000008;  // White queen (d1)
    bb->queens[1] = 0x0800000000000000;  // Black queen (d8)
}

void initialize_kings(Bitboards *bb){
    // Kings
    bb->kings[0] = 0x0000000000000010;   // White king (e1)
    bb->kings[1] = 0x1000000000000000;   // Black king (e8)
}

// void print_bitboard(U64 bitboard) {
//     for (int rank = 0; rank <= 8; rank++) { // Iterate from rank 7 (8th rank) to rank 0 (1st rank)
//         for (int file = 0; file <= 8; file++) {
//             int square = rank * 8 + file; // Calculate the square index
//             printf("%c ", (bitboard & (1ULL << square)) ? '1' : '.');
//         }
//         printf("\n");
//     }
//     printf("\n");
// }


void print_bitboard(U64 bitboard) {
    printf("  a b c d e f g h\n"); // Print file headers
    for (int rank = 7; rank >= 0; rank--) { // Iterate over ranks from 8 to 1
        printf("%d ", rank + 1); // Print rank number
        for (int file = 0; file < 8; file++) { // Iterate over files from a to h
            int square = rank * 8 + file; // Calculate square index
            if (bitboard & (1ULL << square)) { // Check if the bit is set
                printf("1 "); // Bit is set (piece exists)
            } else {
                printf(". "); // Bit is not set (empty square)
            }
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n\n"); // Print file headers again for clarity
}