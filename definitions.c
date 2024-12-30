#include <stdio.h>
#include "definitions.h"

// Convert a square index (0-63) to algebraic notation (e.g., 0 -> "a8")
// using C's implicit ASCII conversion we can use the calculations from 
// ranks and files to get the notation 
void square_to_algebraic(int square, char *notation) {
    notation[0] = 'a' + (square % FILE_COUNT); // File (a-h)
    notation[1] = '8' - (square / FILE_COUNT); // Rank (8-1)
    notation[2] = '\0';
}

// Convert algebraic notation (e.g., "a8") to square index (0-63)
int algebraic_to_square(const char *square) {
    int file = square[0] - 'a';        // Map 'a'-'h' to 0-7
    int rank = square[1] - '1';        // Map '1'-'8' to 0-7

    return (7 - rank) * 8 + (7 - file); // Calculate bitboard index for big-endian mapping
}