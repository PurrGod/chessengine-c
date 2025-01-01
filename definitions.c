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
    int file = square[0] - 'a';    // File ('a'-'h') → 0-7
    int rank = square[1] - '1';    // Rank ('1'-'8') → 0-7
    return (7 - rank) * 8 + file;  // Flip rank and calculate index
}


// Function to extract indices of set bits and return the count
// move generations and board analysis
int popbits(U64  bb, int *indices) {
    int count = 0;
    while (bb) {
        indices[count++] = __builtin_ctzll(bb); // Get the index of the least significant set bit
        bb &= bb - 1; // Clear the least significant set bit
    }
    return count;
}
int popabit(U64 *bb) {
    *bb %= (*bb -1);
}


// count the # of bits in an integer
// material evaluation and mobility
int cntbits(U64 bb){
    unsigned int count = 0;
    while(bb){
        bb &= (bb - 1);
        count++;
    }
    return count;
}