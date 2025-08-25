#include "make_moves.h"
#include "definitions.h"


// save the current board state so we can undo later
static void set_history(Bitboards * bb, int move) {
    bb->history[bb->hisPly].castlePerm = bb->castlePerm;
    bb->history[bb->hisPly].enPas = bb->enPas;
    bb->history[bb->hisPly].posKey = bb->posKey;
    bb->history[bb->hisPly].fiftyMove = bb->fiftyMove;
    bb->history[bb->hisPly].move = move;
    bb->hisPly++;
    bb->ply++;
}

static void capture_move(Bitboards * bb, int m) {
    // capture takes couple of steps
    // 1. remove the enemy piece bit off
    // 2. flip the current piece's bit off
    // 3. flip the current piece's bit on at the proper square

    // in between all of this, any updates will require XOR'ing the key with
    // the appropriate piece properties.

    // for example, if we capture a pawn on d5 with a knight, we would have to 
    // 1. remove the knight, XOR our knight
    // 2. remove enemy piece or pawn, XOR enemy pawn
    // 3. place our knight at appr. position, XOR knight with proper square
    


}

// this function shall process the move integer into actual update
// this function will be used in recursive calls of a future
// search function which searches the different nodes/moves

void make_move(Bitboards * bb, int m) {
    // make history move
    set_history(bb, m);

    // extract all data from the int move
    int to = MOVE_TO(m);
    int from = MOVE_FROM(m);
    int cappt = MOVE_CAPTURED(m);
    int prompt = MOVE_PROMOTION(m);
    int side = (bb->ply % 2 != 0) ? BLACK : WHITE;

    

}