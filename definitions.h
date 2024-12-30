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

enum Squares {
    H1, G1, F1, E1, D1, C1, B1, A1,
    H2, G2, F2, E2, D2, C2, B2, A2,
    H3, G3, F3, E3, D3, C3, B3, A3,
    H4, G4, F4, E4, D4, C4, B4, A4,
    H5, G5, F5, E5, D5, C5, B5, A5,
    H6, G6, F6, E6, D6, C6, B6, A6,
    H7, G7, F7, E7, D7, C7, B7, A7,
    H8, G8, F8, E8, D8, C8, B8, A8
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

