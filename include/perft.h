#ifndef PERFT_H
#define PERFT_H

#include "definitions.h"

// --- Function Prototype ---
// This tells other files that a function named "perft" exists,
// what arguments it takes, and what it returns.
U64 perft(Bitboards *bb, int depth);

#endif // PERFT_H