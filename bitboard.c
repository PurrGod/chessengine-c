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

// Print a bitboard as a chessboard for debugging
void print_bitboard(U64 bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            printf("%c ", (bitboard & (1ULL << square)) ? '1' : '.');
        }
        printf("\n");
    }
    printf("\n");
}
