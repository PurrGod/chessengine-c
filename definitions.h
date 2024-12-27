#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

//unsigned long long
typedef unsigned long long U64;

// Board dimensions
#define BOARD_SIZE 64
#define FILE_COUNT 8
#define RANK_COUNT 8

// Colors
#define WHITE 0
#define BLACK 1

// Piece types
enum Pieces {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE // For empty squares
};

// Square constants (a8 = 0, h1 = 63)
enum Squares {
    A8 = 0, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1
};

// Directions for move generation
enum Directions {
    NORTH = 8,
    SOUTH = -8,
    EAST = 1,
    WEST = -1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_EAST = -7,
    SOUTH_WEST = -9
};

// Utility macros for bitwise operations
#define BIT(n) (1ULL << (n)) // Create a bitmask for the nth square
#define SET_BIT(b, n) ((b) |= BIT(n)) // Set the nth bit in b
#define CLEAR_BIT(b, n) ((b) &= ~BIT(n)) // Clear the nth bit in b
#define IS_SET(b, n) ((b) & BIT(n)) // Check if the nth bit is set in b


#endif // DEFINITIONS_H

