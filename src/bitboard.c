#include <stdio.h>
#include "bitboard.h"
#include "hashkeys.h"
#include <ctype.h>
#include <string.h>
#include "attack.h"

// Initialize bitboards to default starting position
void initialize_bitboards(Bitboards *bb) {
    initialize_pawns(bb);
    initialize_knights(bb);
    initialize_bishops(bb);
    initialize_rook(bb);
    initialize_queens(bb);
    initialize_kings(bb);

    // Occupied squares
    bb->occupied[0] = bb->pawns[0] | bb->knights[0] | bb->bishops[0] | bb->rooks[0] | bb->queens[0] | bb->kings[0];
    bb->occupied[1] = bb->pawns[1] | bb->knights[1] | bb->bishops[1] | bb->rooks[1] | bb->queens[1] | bb->kings[1];

    // All pieces
    bb->all_pieces = bb->occupied[0] | bb->occupied[1];

    // initializing the board state values
    bb->side = WHITE;
    bb->enPas = NO_SQ;
    bb->ply = 0;
    bb->hisPly = 0;
    bb->fiftyMove = 0;

    // all castle rights are enabled in the starting position
    bb->castlePerm = WKCA | WQCA | BKCA | BQCA;
    bb->posKey = zobrist_hashing_posKey(bb);
}

void initialize_pawns(Bitboards *bb){
    // Pawns
    bb->pawns[0] = 0x000000000000FF00;  // White pawns
    bb->pawns[1] = 0x00FF000000000000;  // Black pawns
}

void initialize_knights(Bitboards *bb){
    // Knights
    bb->knights[0] = 0x0000000000000042; // White knights (b1, g1)
    bb->knights[1] = 0x4200000000000000; // Black knights (b8, g8)
}

void initialize_bishops(Bitboards *bb){
    // Bishops
    bb->bishops[0] = 0x0000000000000024; // White bishops (c1, f1)
    bb->bishops[1] = 0x2400000000000000; // Black bishops (c8, f8)
}

void initialize_rook(Bitboards *bb){
    // Rooks
    bb->rooks[0] = 0x0000000000000081;   // White rooks (a1, h1)
    bb->rooks[1] = 0x8100000000000000;   // Black rooks (a8, h8)
}

void initialize_queens(Bitboards *bb){
    // Queens
    bb->queens[0] = 0x0000000000000008;  // White queen (d1)
    bb->queens[1] = 0x0800000000000000;  // Black queen (d8)
}

void initialize_kings(Bitboards *bb){
    // Kings
    bb->kings[0] = 0x0000000000000010;   // White king (e1)
    bb->kings[1] = 0x1000000000000000;   // Black king (e8)
}

// Print a bitboard in a human-readable format

void print_bitboard(U64 bitboard) {
    printf("  a b c d e f g h\n");
    for (int rank = 0; rank < 8; rank++) {
        printf("%d ", 8 - rank);
        for (int file = 0; file < 8; file++) {
            // Use the same, correct logic as print_square_indices
            int square = (7 - rank) * 8 + file;
            char c = (bitboard & (1ULL << square)) ? '1' : '.';
            printf("%c ", c);
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n\n");
}


void print_square_indices() {
    // This text now correctly describes your board representation.
    printf("Bitboard square indices (MSB = H8 = 63, LSB = A1 = 0):\n\n");

    for (int rank = 0; rank < 8; rank++) {
        printf("%d ", 8 - rank); // Rank label (8 to 1)
        for (int file = 0; file < 8; file++) {
            // This logic correctly calculates square indices for A1=0.
            int square = (7 - rank) * 8 + file; // Flip rank to match bitboard indexing
            printf("%2d ", square);
        }
        printf("\n");
    }
    printf("  a  b  c  d  e  f  g  h\n\n");
}

void print_board(const Bitboards *bb) {
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            char piece_char = '.'; // Default to empty square

            // Check each piece type for both colors
            if (isset(bb->pawns[WHITE], square)) piece_char = 'P';
            else if (isset(bb->knights[WHITE], square)) piece_char = 'N';
            else if (isset(bb->bishops[WHITE], square)) piece_char = 'B';
            else if (isset(bb->rooks[WHITE], square)) piece_char = 'R';
            else if (isset(bb->queens[WHITE], square)) piece_char = 'Q';
            else if (isset(bb->kings[WHITE], square)) piece_char = 'K';
            else if (isset(bb->pawns[BLACK], square)) piece_char = 'p';
            else if (isset(bb->knights[BLACK], square)) piece_char = 'n';
            else if (isset(bb->bishops[BLACK], square)) piece_char = 'b';
            else if (isset(bb->rooks[BLACK], square)) piece_char = 'r';
            else if (isset(bb->queens[BLACK], square)) piece_char = 'q';
            else if (isset(bb->kings[BLACK], square)) piece_char = 'k';

            printf(" %c |", piece_char);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n\n");
    printf("Side to move: %s\n", (bb->side == WHITE) ? "White" : "Black");
    printf("Castling: %c%c%c%c\n",
           (bb->castlePerm & WKCA) ? 'K' : '-',
           (bb->castlePerm & WQCA) ? 'Q' : '-',
           (bb->castlePerm & BKCA) ? 'k' : '-',
           (bb->castlePerm & BQCA) ? 'q' : '-');
    
    char enPasSq[3] = "-";
    if (bb->enPas != NO_SQ) {
        square_to_algebraic(bb->enPas, enPasSq);
    }
    printf("En Passant: %s\n", enPasSq);
    printf("Position Key: %llx\n", bb->posKey);
}


void parse_fen(Bitboards *bb, const char *fen) {
    // 1. Clear all bitboards and reset game state
    memset(bb, 0, sizeof(Bitboards));

    int rank = 7;
    int file = 0;
    int fen_index = 0;
    char c;

    // 2. Parse Piece Placement
    while ((c = fen[fen_index]) != ' ' && c != '\0') {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += c - '0';
        } else {
            int square = rank * 8 + file;
            if (c == 'P') setbit(bb->pawns[WHITE], square);
            else if (c == 'N') setbit(bb->knights[WHITE], square);
            else if (c == 'B') setbit(bb->bishops[WHITE], square);
            else if (c == 'R') setbit(bb->rooks[WHITE], square);
            else if (c == 'Q') setbit(bb->queens[WHITE], square);
            else if (c == 'K') setbit(bb->kings[WHITE], square);
            else if (c == 'p') setbit(bb->pawns[BLACK], square);
            else if (c == 'n') setbit(bb->knights[BLACK], square);
            else if (c == 'b') setbit(bb->bishops[BLACK], square);
            else if (c == 'r') setbit(bb->rooks[BLACK], square);
            else if (c == 'q') setbit(bb->queens[BLACK], square);
            else if (c == 'k') setbit(bb->kings[BLACK], square);
            file++;
        }
        fen_index++;
    }
    fen_index++; // Skip the space

    // 3. Parse Active Color
    bb->side = (fen[fen_index] == 'w') ? WHITE : BLACK;
    fen_index += 2; // Skip color and space

    // 4. Parse Castling Availability
    while ((c = fen[fen_index]) != ' ' && c != '\0') {
        switch (c) {
            case 'K': bb->castlePerm |= WKCA; break;
            case 'Q': bb->castlePerm |= WQCA; break;
            case 'k': bb->castlePerm |= BKCA; break;
            case 'q': bb->castlePerm |= BQCA; break;
        }
        fen_index++;
    }
    fen_index++; // Skip the space

    // 5. Parse En Passant Target Square
    if (fen[fen_index] != '-') {
        bb->enPas = algebraic_to_square(&fen[fen_index]);
    } else {
        bb->enPas = NO_SQ;
    }
    // (We skip parsing halfmove and fullmove for now as they aren't needed for Perft)

    // Finally, update aggregate bitboards
    for (int i = 0; i < 2; i++) {
        bb->occupied[i] = bb->pawns[i] | bb->knights[i] | bb->bishops[i] | bb->rooks[i] | bb->queens[i] | bb->kings[i];
    }
    bb->all_pieces = bb->occupied[WHITE] | bb->occupied[BLACK];
    bb->posKey = zobrist_hashing_posKey(bb);
}


// In src/bitboard.c

// Helper to get the character for a piece on a square
static char get_piece_char(const Bitboards *bb, int square) {
    if (isset(bb->pawns[WHITE], square)) return 'P';
    if (isset(bb->knights[WHITE], square)) return 'N';
    if (isset(bb->bishops[WHITE], square)) return 'B';
    if (isset(bb->rooks[WHITE], square)) return 'R';
    if (isset(bb->queens[WHITE], square)) return 'Q';
    if (isset(bb->kings[WHITE], square)) return 'K';
    if (isset(bb->pawns[BLACK], square)) return 'p';
    if (isset(bb->knights[BLACK], square)) return 'n';
    if (isset(bb->bishops[BLACK], square)) return 'b';
    if (isset(bb->rooks[BLACK], square)) return 'r';
    if (isset(bb->queens[BLACK], square)) return 'q';
    if (isset(bb->kings[BLACK], square)) return 'k';
    return ' '; // Should not happen on an occupied square
}

void board_to_fen(Bitboards *bb, char *fen_str) {
    int empty_count = 0;
    int char_index = 0;

    // 1. Piece Placement
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (isset(bb->all_pieces, square)) {
                if (empty_count > 0) {
                    fen_str[char_index++] = (char)('0' + empty_count);
                    empty_count = 0;
                }
                fen_str[char_index++] = get_piece_char(bb, square);
            } else {
                empty_count++;
            }
        }
        if (empty_count > 0) {
            fen_str[char_index++] = (char)('0' + empty_count);
        }
        if (rank > 0) {
            fen_str[char_index++] = '/';
        }
        empty_count = 0;
    }

    // 2. Active Color
    fen_str[char_index++] = ' ';
    fen_str[char_index++] = (bb->side == WHITE) ? 'w' : 'b';
    fen_str[char_index++] = ' ';

    // 3. Castling Availability
    int castle_char_written = 0;
    if (bb->castlePerm & WKCA) { fen_str[char_index++] = 'K'; castle_char_written = 1; }
    if (bb->castlePerm & WQCA) { fen_str[char_index++] = 'Q'; castle_char_written = 1; }
    if (bb->castlePerm & BKCA) { fen_str[char_index++] = 'k'; castle_char_written = 1; }
    if (bb->castlePerm & BQCA) { fen_str[char_index++] = 'q'; castle_char_written = 1; }
    if (!castle_char_written) {
        fen_str[char_index++] = '-';
    }
    fen_str[char_index++] = ' ';

    // 4. En Passant Target Square
    if (bb->enPas != NO_SQ) {
        square_to_algebraic(bb->enPas, &fen_str[char_index]);
        char_index += 2;
    } else {
        fen_str[char_index++] = '-';
    }
    fen_str[char_index++] = ' ';

    // 5. Halfmove Clock and Fullmove Number (using placeholders)
    // We can use sprintf for simplicity here.
    sprintf(&fen_str[char_index], "%d %d", bb->fiftyMove, (bb->ply / 2) + 1);
    
    // Null terminate the final string
    // sprintf does this for us.
}