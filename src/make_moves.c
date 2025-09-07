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
    int movinPiece = get_piece_on_square(bb, from, side);
    U64 * piece_bb = get_piece_bitboard(bb, movinPiece);

    clearbit(*piece_bb, from);
    setbit(*piece_bb, to);
    
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

        clearbit(bb->occupied[1 - side], to);


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
                setbit(bb->occupied[BLACK], D8);
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

    if (cappt == wRook) {
        if (to == H1) { bb->castlePerm &= ~WKCA; }
        else if (to == A1) { bb->castlePerm &= ~WQCA; }
    } else if (cappt == bRook) {
        if (to == H8) { bb->castlePerm &= ~BKCA; }
        else if (to == A8) { bb->castlePerm &= ~BQCA; }
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


// reverse all changes made from a move integer using bb.history
// Add this function to make_moves.c

void unmake_move(Bitboards *bb) {
    // Decrement the history ply to point to the last move made
    bb->hisPly--;
    bb->ply--;

    // RESTORE PREVIOUS STATE
    // Restore all the board state variables from the history array.

    int move = bb->history[bb->hisPly].move;
    bb->side = bb->history[bb->hisPly].side;
    bb->fiftyMove = bb->history[bb->hisPly].fiftyMove;
    bb->enPas = bb->history[bb->hisPly].enPas;
    bb->castlePerm = bb->history[bb->hisPly].castlePerm;
    bb->posKey = bb->history[bb->hisPly].posKey;
    // assuming all the revesals are accurate, we simply don't need to 
    // recalculate or reXor all the operations, we can just assign previous
    // posKey of previous board state.

    // extract previous move details
    int from = MOVE_FROM(move);
    int to = MOVE_TO(move);
    int captured = MOVE_CAPTURED(move);
    int promoted = MOVE_PROMOTION(move);

    // REVERSE THE PIECE MOVEMENT
    
    // Identify the piece that moved. If it was a promotion, the piece on the 'to'
    // square is the promoted piece, but the piece we need to move back is a pawn.
    int moving_piece;
    if (promoted != EMPTY) {
        moving_piece = (bb->side == WHITE) ? wPawn : bPawn;
        // Remove the promoted piece from the 'to' square
        U64 *promoted_bb_ptr = get_piece_bitboard(bb, promoted);
        clearbit(*promoted_bb_ptr, to);
    } else {
        // We are moving back the back so the moving piece is "to" square
        moving_piece = get_piece_on_square(bb, to, bb->side);
    }

    // Move the piece back from 'to' to 'from'
    U64 *piece_bb_ptr = get_piece_bitboard(bb, moving_piece);
    setbit(*piece_bb_ptr, from);
    clearbit(*piece_bb_ptr, to);

    // Update the bitboards
    setbit(bb->occupied[bb->side], from);
    clearbit(bb->occupied[bb->side], to);
    setbit(bb->all_pieces, from);
    clearbit(bb->all_pieces, to);

    // UNDO SPECIAL MOVES

    // If it was a capture, put the captured piece back on the 'to' square
    if (captured != EMPTY) {
        // For en passant, the captured piece is NOT on the 'to' square
        if (move & MOVE_IS_ENPASSANT) {
            int cap_sq = (bb->side == WHITE) ? to - 8 : to + 8;
            setbit(bb->pawns[!bb->side], cap_sq);
            setbit(bb->occupied[!bb->side], cap_sq);
            setbit(bb->all_pieces, cap_sq);
        } else {
            // For a standard capture
            U64 *captured_bb_ptr = get_piece_bitboard(bb, captured);
            setbit(*captured_bb_ptr, to);
            setbit(bb->occupied[!bb->side], to);
            setbit(bb->all_pieces, to);
        }
    }

    // If it was a castle, move the rook back to its home square
    if (move & MOVE_IS_CASTLE) {
        switch (to) {
            case G1: // White Kingside
                setbit(bb->rooks[WHITE], H1);
                clearbit(bb->rooks[WHITE], F1);
                setbit(bb->occupied[WHITE], H1);
                clearbit(bb->occupied[WHITE], F1);
                setbit(bb->all_pieces, H1);
                clearbit(bb->all_pieces, F1);
                break;
            case C1: // White Queenside
                setbit(bb->rooks[WHITE], A1);
                clearbit(bb->rooks[WHITE], D1);
                setbit(bb->occupied[WHITE], A1);
                clearbit(bb->occupied[WHITE], D1);
                setbit(bb->all_pieces, A1);
                clearbit(bb->all_pieces, D1);
                break;
            case G8: // Black Kingside
                setbit(bb->rooks[BLACK], H8);
                clearbit(bb->rooks[BLACK], F8);
                setbit(bb->occupied[BLACK], H8);
                clearbit(bb->occupied[BLACK], F8);
                setbit(bb->all_pieces, H8);
                clearbit(bb->all_pieces, F8);
                break;
            case C8: // Black Queenside
                setbit(bb->rooks[BLACK], A8);
                clearbit(bb->rooks[BLACK], D8);
                setbit(bb->occupied[BLACK], A8);
                clearbit(bb->occupied[BLACK], D8);
                setbit(bb->all_pieces, A8);
                clearbit(bb->all_pieces, D8);
                break;
        }
    }
}


// is repetition check:
// return true if count = 3
// iterate backwards through the history array.

int is_repetition(Bitboards * bb) {
    int count = 1;
    U64 currPosKey = bb->posKey;
    for (int i = bb->hisPly - 1; i >=0; i--) {
        // go backwards through array
        if (bb->history[i].posKey == currPosKey){count++;}
        if (count >= 3){return 1;}

        // check for 50 rule move reset
        if (bb->history[i].fiftyMove == 0) {
            break;
        }
    }

    return 0; // if the loop finishes, we haven't found the repetition -> return draw.
}