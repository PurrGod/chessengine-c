#include <stdlib.h>
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

/*
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
*/


// this function shall process the move integer into actual update
// this function will be used in recursive calls of a future
// search function which searches the different nodes/moves
// This function always updates each move as a silent move
// and then updates the extra special conditions such as 
// promotion and castling etc.

// Don't forget to update the piece bitboard, occupied bitboard and all pieces bitboard
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

    // Check for enpassant move
    // if white, the disappeaaring pawn after enpassant is
    // to_sq - 8
    // if black, the white pawn we need to clear bit of is
    // to_sq + 8

    // 
    if (m & MOVE_IS_ENPASSANT) {
        if (movinPiece == wPawn) {
            int cap_sq = to - 8;

            // update board
            clearbit(bb->pawns[BLACK], cap_sq);
            clearbit(bb->occupied[BLACK], cap_sq);
            clearbit(bb->all_pieces, cap_sq);
             // update pos key for enpassant
            bb->posKey ^= PieceKeys[bPawn][cap_sq];
        } else if (movinPiece == bPawn) {
            int cap_sq = to + 8;

            clearbit(bb->pawns[WHITE], cap_sq);
            clearbit(bb->occupied[WHITE], cap_sq);
            clearbit(bb->all_pieces, cap_sq);

            bb->posKey ^= PieceKeys[wPawn][cap_sq];
        }
    }

   

    if (m & MOVE_IS_CASTLE) {
        // which castle move it is
        switch (to) {
            case G1:
                // update rook from H1 to F1
                clearbit(bb->rooks[WHITE], H1);
                setbit(bb->rooks[WHITE], F1);
                // update xor
                bb->posKey ^= PieceKeys[wRook][H1];
                bb->posKey ^= PieceKeys[wRook][F1];

                clearbit(bb->occupied[WHITE], H1);
                setbit(bb->occupied[WHITE], F1);
                clearbit(bb->all_pieces, H1);
                setbit(bb->all_pieces, F1);
                break;
            case C1:
                clearbit(bb->rooks[WHITE], A1);
                setbit(bb->rooks[WHITE], D1);
                // update xor
                bb->posKey ^= PieceKeys[wRook][A1];
                bb->posKey ^= PieceKeys[wRook][D1];

                clearbit(bb->occupied[WHITE], A1);
                setbit(bb->occupied[WHITE], D1);
                clearbit(bb->all_pieces, A1);
                setbit(bb->all_pieces, D1);
                break;        
            case G8:
                clearbit(bb->rooks[BLACK], H8);
                setbit(bb->rooks[BLACK], F8);
                // update xor
                bb->posKey ^= PieceKeys[bRook][H8];
                bb->posKey ^= PieceKeys[bRook][F8];

                clearbit(bb->occupied[BLACK], H8);
                setbit(bb->occupied[BLACK], F8);
                clearbit(bb->all_pieces, H8);
                setbit(bb->all_pieces, F8);
                break;
            case C8:
                clearbit(bb->rooks[BLACK], A8);
                setbit(bb->rooks[BLACK], D8);
                // update xor
                bb->posKey ^= PieceKeys[bRook][A8];
                bb->posKey ^= PieceKeys[bRook][D8];

                clearbit(bb->occupied[BLACK], A8);
                clearbit(bb->occupied[BLACK], D8);
                clearbit(bb->all_pieces, A8);
                setbit(bb->all_pieces, D8);
                break; 
        }
    }

    bb->posKey ^= CastleKeys[bb->castlePerm];

    switch (movinPiece) {
        case wKing:
            bb->castlePerm &= ~(WKCA | WQCA);
            break;
        case bKing:
            bb->castlePerm &= ~(BKCA | BQCA);
            break;
        case wRook:
            if (from == H1){bb->castlePerm &= ~WKCA;};
            if (from == A1){bb->castlePerm &= ~WQCA;};
            break;
        case bRook:
            if (from == H8){bb->castlePerm &= ~BKCA;};
            if (from == A8){bb->castlePerm &= ~BQCA;};
            break;
    }

    bb->posKey ^= CastleKeys[bb->castlePerm];

    // end of function we need to update castle perms, enpassant perms
    // as well as fifty move and finally flip the side -> side ^= 1

    // erase previous EN_PAS since it only counts for one move
    if (bb->enPas != NO_SQ) {
        bb->posKey ^= EPFileKey[bb->enPas % 8];
    }
    bb->enPas = NO_SQ;

    // check if a possible enpassant square can be made
    // happens when a pawn moves twice in same move (double push)
    if (movinPiece == wPawn || movinPiece == bPawn){
        if (abs(to - from) == 16) {
            if (movinPiece == wPawn) {
                bb->enPas = to - 8;
            } else if (movinPiece == bPawn) {
                bb->enPas = to + 8;
            }
            bb->posKey ^= EPFileKey[bb->enPas % 8];
        }
    }

    // fifty move counter
    // no capture or pawn movement for 50 moves
    if (movinPiece == bPawn || movinPiece == wPawn || cappt != EMPTY){
        bb->fiftyMove = 0;
    } else {
        bb->fiftyMove++;
    }

    // update side
    bb->side ^= 1;
    bb->posKey ^= Sidekey;
}