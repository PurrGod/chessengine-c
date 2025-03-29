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
    int file = 7 - (square[0] - 'a');   // Flip file
    int rank = square[1] - '1';         // Bottom = 0
    return rank * 8 + file;
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

//isolate the lsb
//invert it with complement and add one
//take the xor to flip the lsb
U64 popabit(U64 *bb, int *lsb_index) {
    if (*bb == 0){
        *lsb_index = -1;
        return 0;
    }

    U64 lsb = *bb & -*bb;
    *lsb_index = __builtin_ctzll(lsb); // Get the index of the LSB
    *bb ^= lsb;
    return *bb;
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