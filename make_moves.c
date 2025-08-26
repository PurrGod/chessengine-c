#include "make_moves.h"
#include "definitions.h"
#include "hashkeys.h"


// save the current board state so we can undo later
static void set_history(Bitboards * bb, int move) {
    bb->history[bb->hisPly].castlePerm = bb->castlePerm;
    bb->history[bb->hisPly].enPas = bb->enPas;
    bb->history[bb->hisPly].posKey = bb->posKey;
    bb->history[bb->hisPly].fiftyMove = bb->fiftyMove;
    bb->history[bb->hisPly].move = move;
    bb->history[bb->hisPly].side = bb->side;
    bb->hisPly++;
    bb->ply++;
}

// static void capture_move(Bitboards * bb, int m) {
//     // capture takes couple of steps
//     // 1. remove the enemy piece bit off
//     // 2. flip the current piece's bit off
//     // 3. flip the current piece's bit on at the proper square

//     // in between all of this, any updates will require XOR'ing the key with
//     // the appropriate piece properties.

//     // for example, if we capture a pawn on d5 with a knight, we would have to 
//     // 1. remove the knight, XOR our knight
//     // 2. remove enemy piece or pawn, XOR enemy pawn
//     // 3. place our knight at appr. position, XOR knight with proper square
    

// }

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
    int side = bb->side;

    // piece that is moving, we can get that by using the from parameter
    // use the switch statement to then retrieve the bb array
    int movinPiece = get_piece_on_square(bb, from, side);
    switch(movinPiece) {
        case wPawn:
            clearbit(bb->pawns[WHITE], from);
            setbit(bb->pawns[WHITE], to);
            break;
        case wKnight:
            clearbit(bb->knights[WHITE], from);
            setbit(bb->knights[WHITE], to);
            break;
        case wBishop:
            clearbit(bb->bishops[WHITE], from);
            setbit(bb->bishops[WHITE], to);
            break;
        case wRook:
            clearbit(bb->rooks[WHITE], from);
            setbit(bb->rooks[WHITE], to);
            break;
        case wQueen:
            clearbit(bb->queens[WHITE], from);
            setbit(bb->queens[WHITE], to);
            break;
        case wKing:
            clearbit(bb->kings[WHITE], from);
            setbit(bb->kings[WHITE], to);
            break;

        // BLACK pieces
        case bPawn:
            clearbit(bb->pawns[BLACK], from);
            setbit(bb->pawns[BLACK], to);
            break;
        case bKnight:
            clearbit(bb->knights[BLACK], from);
            setbit(bb->knights[BLACK], to);
            break;
        case bBishop:
            clearbit(bb->bishops[BLACK], from);
            setbit(bb->bishops[BLACK], to);
            break;
        case bRook:
            clearbit(bb->rooks[BLACK], from);
            setbit(bb->rooks[BLACK], to);
            break;
        case bQueen:
            clearbit(bb->queens[BLACK], from);
            setbit(bb->queens[BLACK], to);
            break;
        case bKing:
            clearbit(bb->kings[BLACK], from);
            setbit(bb->kings[BLACK], to);
            break;        
    }

    // no matter what move, the from and to bits 
    // have to be updated and the xor needs to done
    // update hash
    bb->posKey ^= PieceKeys[movinPiece][from];
    bb->posKey ^= PieceKeys[movinPiece][to];

    // update the bitboards
    clearbit(bb->occupied[side], from);
    setbit(bb->occupied[side], to);
    clearbit(bb->all_pieces, from);
    setbit(bb->all_pieces, to);

    // now we will handle the special flags such as:
    //      *if capture
    //      *if promotions
    //      *if castle
    
    if (cappt != EMPTY) {
        U64 *capbb = get_piece_bitboard(bb, cappt);
        clearbit(*capbb, to);

        // update hashing
        bb->posKey ^= PieceKeys[cappt][to];
    }

    if (prompt != EMPTY) {
        // For promotion, we need to change the piece type altogether
        // requires us to clear bit from prior piece type, set bit for new piece type
        
        // removing prior piece type
        U64 * pawn_bb_pointer = get_piece_bitboard(bb, movinPiece);
        clearbit(*pawn_bb_pointer, to);

        // change the piece type to prom_pt
        U64 * promPT_bb = get_piece_bitboard(bb, prompt);
        setbit(*promPT_bb, to);

        // update hash
        bb->posKey ^= PieceKeys[movinPiece][to];
        bb->posKey ^= PieceKeys[prompt][to];
    }

    if (m & MOVE_IS_CASTLE) {

    }

}