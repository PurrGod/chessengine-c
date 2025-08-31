#include "definitions.h"
#include "evaluate.h"
#include <math.h>

// piece square tables
static const int mg_PawnPST[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0, -20, -20,   0,   0,   0,
    0,   0,   0,   5,   5,   0,  -5,   5,
    5,   5,  10,  20,  20,   5,   5,   5,
   10,  10,  10,  30,  30,  10,  10,  10,
   35,  35,  35,  40,  40,  35,  25,  25,
   50,  50,  50,  50,  50,  50,  50,  50,
   80,  80,  80,  80,  80,  80,  80,  80
};

// Correctly ordered for A1=0. Rank 1 is first.
static const int eg_PawnPST[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0, -20, -20,   0,   0,   0,
    0,   0,   0,   5,   5,   0,  -5,   5,
    5,   5,  10,  20,  20,   5,   5,   5,
   30,  30,  30,  30,  30,  30,  30,  30,
   65,  65,  65,  60,  60,  65,  65,  65,
   90,  90,  90,  90,  90,  90,  90,  90,
  150, 150, 150, 150, 150, 150, 150, 150
};


// Middlegame Knight: Values are high for central outposts, reflecting its power in a complex position.
static const int mg_KnightPST[64] = {
  -50, -40, -30, -30, -30, -30, -40, -50,
  -40, -20,   0,   5,   5,   0, -20, -40,
  -30,   5,  15,  25,  25,  15,   5, -30,
  -30,   0,  25,  50,  50,  25,   0, -30,
  -30,   5,  25,  50,  50,  25,   5, -30,
  -30,   0,  15,  25,  25,  15,   0, -30,
  -40, -20,   0,   0,   0,   0, -20, -40,
  -50, -40, -30, -30, -30, -30, -40, -50
};

// Endgame Knight: The peak values are slightly lower and more spread out.
// The knight is still good in the center, but less dominant than in the middlegame.
static const int eg_KnightPST[64] = { 
  -50, -40, -30, -30, -30, -30, -40, -50, 
  -40, -20,   0,   0,   0,   0, -20, -40,
  -30,   0,  10,  15,  15,  10,   0, -30,
  -30,   5,  15,  20,  20,  15,   5, -30,
  -30,   5,  15,  20,  20,  15,   5, -30,
  -30,   0,  10,  15,  15,  10,   0, -30,
  -40, -20,   0,   0,   0,   0, -20, -40,
  -50, -40, -30, -30, -30, -30, -40, -50
};


// Middlegame Rook: Small bonuses for staying on the back rank, ready to seize a file.
// Penalizes rooks on bad squares like a3 or h6.
static const int mg_RookPST[64] = {
    0,   0,   0,   5,   5,   0,   0,   0,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
   -5,   0,   0,   0,   0,   0,   0,  -5,
    5,  10,  10,  10,  10,  10,  10,   5,
    0,   0,   0,   0,   0,   0,   0,   0
};

// Endgame Rook: The 7th rank is gold. Values are much higher as the rook becomes a key attacking piece.
static const int eg_RookPST[64] = {
    0,   0,   0,   0,   0,   0,   0,  0,
    5,  10,  10,  10,  10,  10,  10,  5,
   -5,   0,   0,   0,   0,   0,   0, -5,
   -5,   0,   0,   0,   0,   0,   0, -5,
   -5,   0,   0,   0,   0,   0,   0, -5,
   -5,   0,   0,   0,   0,   0,   0, -5,
   25,  25,  25,  25,  25,  25,  25, 25, // Huge bonus for the 7th rank
    0,   0,   0,   5,   5,   0,   0,  0
};

// Middlegame Bishop: Encourages development to active squares and fianchettos.
static const int mg_BishopPST[64] = {
  -20, -10, -10, -10, -10, -10, -10, -20,
  -10,   5,   0,   0,   0,   0,   5, -10,
  -10,  10,  10,  10,  10,  10,  10, -10,
  -10,   0,  10,  10,  10,  10,   0, -10,
  -10,   5,   5,  10,  10,   5,   5, -10,
  -10,   0,   5,  10,  10,   5,   0, -10,
  -10,   0,   0,   0,   0,   0,   0, -10,
  -20, -10, -10, -10, -10, -10, -10, -20
};

// Endgame Bishop: The values increase. In an open endgame, a bishop's long-range power is amplified.
static const int eg_BishopPST[64] = {
  -20, -10, -10, -10, -10, -10, -10, -20,
  -10,  10,   0,   0,   0,  10,  10, -10,
  -10,   0,   0,  10,  10,  10,   0, -10,
  -10,   5,   5,  10,  10,   5,   5, -10,
  -10,   0,  10,  10,  10,  10,   0, -10,
  -10,  10,  10,  10,  10,  10,   5, -10,
  -10,   5,   0,   0,   0,   0,   5, -10,
  -20, -10, -10, -10, -10, -10, -10, -20
};

// Middlegame King: Safety is the ONLY priority. This table heavily encourages
// the king to castle and stay out of the center.
static const int mg_king_pst[64] = {
   20,  30,  10,   0,   0,  10,  30,  20,
   20,  20,   0,   0,   0,   0,  20,  20,
  -10, -20, -20, -20, -20, -20, -20, -10,
  -20, -30, -30, -40, -40, -30, -30, -20,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30,
  -30, -40, -40, -50, -50, -40, -40, -30
};

// Endgame King: Activity is EVERYTHING. The king becomes a powerful attacker.
// This table strongly incentivizes the king to march towards the center.
static const int eg_king_pst[64] = {
  -50, -30, -20, -10, -10, -20, -30, -50,
  -30, -10,   0,  10,  10,   0, -10, -30,
  -20,   0,  20,  30,  30,  20,   0, -20,
  -10,  10,  30,  40,  40,  30,  10, -10,
  -10,  10,  30,  40,  40,  30,  10, -10,
  -20,   0,  20,  30,  30,  20,   0, -20,
  -30, -10,   0,  10,  10,   0, -10, -30,
  -50, -30, -20, -10, -10, -20, -30, -50
};

// Middlegame Queen: Encourages development off the back rank but penalizes
// early, centralized adventures where it can become a target.
static const int mg_QueenPST[64] = {
  -20, -10, -10,  -5,  -5, -10, -10, -20,
  -10,   0,   0,   0,   0,   0,   0, -10,
  -10,   0,   5,   5,   5,   5,   0, -10,
   -5,   0,   5,   5,   5,   5,   0,  -5,
    0,   0,   5,   5,   5,   5,   0,  -5,
  -10,   5,   5,   5,   5,   5,   0, -10,
  -10,   0,   5,   0,   0,   0,   0, -10,
  -20, -10, -10,  -5,  -5, -10, -10, -20
};

// Endgame Queen: Centralization is key. The queen becomes a dominant force,
// combining the power of a rook and bishop to control the open board.
static const int eg_QueenPST[64] = {
  -30, -20, -10, -10, -10, -10, -20, -30,
  -20, -10,   0,   5,   5,   0, -10, -20,
  -10,   0,  10,  15,  15,  10,   0, -10,
  -10,   5,  15,  20,  20,  15,   5, -10,
  -10,   0,  15,  20,  20,  15,   0, -10,
  -10,   5,  10,  15,  15,  10,   5, -10,
  -20, -10,   0,   0,   0,   0, -10, -20,
  -30, -20, -10, -10, -10, -10, -20, -30
};

// Evaluate functions
// tapered position eval function:

// give value to pieces:
static const int mg_piece_val[6] = {100, 310, 350, 500, 900, 0};
static const int eg_piece_val[6] = {100, 310, 350, 500, 900, 0};

static const int *mg_pst_tables[6] = {
    mg_PawnPST,
    mg_KnightPST,
    mg_BishopPST,
    mg_RookPST,
    mg_QueenPST,
    mg_king_pst
};

static const int *eg_pst_tables[6] = {
    eg_PawnPST,
    eg_KnightPST,
    eg_BishopPST,
    eg_RookPST,
    eg_QueenPST,
    eg_king_pst
};

// weights of the pieces
static const int game_phase[13] = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};

int evaluate(Bitboards * bb) {
	;
  
}
