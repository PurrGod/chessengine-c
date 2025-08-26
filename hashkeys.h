// in hashkeys.h

#ifndef HASHKEYS_H
#define HASHKEYS_H

#include "definitions.h"

// Use 'extern' to make these global variables visible to other files
extern U64 PieceKeys[13][64];
extern U64 Sidekey;
extern U64 CastleKeys[16];
extern U64 EPFileKey[8];

// Function Prototypes
void init_hash_keys();
U64 zobrist_hashing_posKey(Bitboards * bb);

#endif // HASHKEYS_H