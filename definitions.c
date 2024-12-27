#include <stdio.h>
#include "definitions.h"

// Convert a square index (0-63) to algebraic notation (e.g., 0 -> "a8")
void square_to_algebraic(int square, char *notation) {
    notation[0] = 'a' + (square % FILE_COUNT); // File (a-h)
    notation[1] = '8' - (square / FILE_COUNT); // Rank (8-1)
    notation[2] = '\0'; // Null-terminate the string
}

// Convert algebraic notation (e.g., "a8") to square index (0-63)
int algebraic_to_square(const char *notation) {
    int file = notation[0] - 'a'; // File (a-h)
    int rank = '8' - notation[1]; // Rank (8-1)
    return rank * FILE_COUNT + file;
}
