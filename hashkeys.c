#include "definitions.h"

//generate position keys to create a unique key for each position
//this is used to check for repeated states
//common technique is to XOR all te keys

U64 PieceKeys[13][64];
U64 Sidekey;
U64 CastleKeys[16];

