#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

//unsigned long long
typedef unsigned long long U64;

//max game moves
#define MAXGAMEMOVES 2048 //half moves

//information for undo moves
typedef struct{
    int move;
    int castlePerm;
    int enPas;
    int fiftyMove;
    U64 posKey;
} S_undo;

typedef struct {
    U64 pawns[2];    // 0 for white, 1 for black
    U64 knights[2];
    U64 bishops[2];
    U64 rooks[2];
    U64 queens[2];
    U64 kings[2];
    U64 occupied[2]; // 0 for white's occupied, 1 for black's occupied
    U64 all_pieces;  // All pieces combined (both white and black)

    int enPas;
    int ply; //number of half moves
    int hisPly;

    //check history[] to see if posKey repeats itself
    U64 posKey; //unique key for that specific position
    int fiftyMove; //status of the fiftymove rule

    int castlePerm; //1001 represents castle permission 

    int pceNum[13]; //stats of piece number
    int bigPce[3];  //pieces that are not pawns
    int majorPce[3]; //rook queen king
    int minorPce[3]; //knight bishop

    S_undo history[MAXGAMEMOVES]; //we have an array of all the moves, we can iterate 
                                  //to anywhere to find all the details regarding the board position

} Bitboards;



// Board dimensions
#define BOARD_SIZE 64
#define FILE_COUNT 8
#define RANK_COUNT 8

// Colors
#define WHITE 0
#define BLACK 1

// Piece types
enum Pieces {
    EMPTY = 0,
    wPawn,
    wKnight,
    wBishop,
    wRook,
    wQueen,
    wKing,
    bPawn,
    bKnight,
    bBishop,
    bRook,
    bQueen,
    bKing,
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

//white and black castling
enum {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

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


void square_to_algebraic(int square, char *notation);
int algebraic_to_square(const char *notation);

#endif // DEFINITIONS_H

