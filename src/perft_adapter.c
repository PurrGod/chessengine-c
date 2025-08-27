// src/perft_adapter.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "definitions.h"
#include "make_moves.h"
#include "attack.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include <ctype.h>
#include "hashkeys.h"

// --- New Function: Parse a UCI move string ---
// Finds the corresponding move integer for a UCI move string like "e2e4" or "a7a8q"
static int parse_move_uci(Bitboards *board, const char *uci_move_str) {
    moveList list;
    generate_all_moves(board, board->side, &list);

    int from_sq = algebraic_to_square(uci_move_str);
    int to_sq = algebraic_to_square(uci_move_str + 2);

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        if (MOVE_FROM(move) == from_sq && MOVE_TO(move) == to_sq) {
            int promo_piece = MOVE_PROMOTION(move);
            if (promo_piece != EMPTY) {
                char promo_char = tolower(uci_move_str[4]);
                if ((promo_char == 'q' && (promo_piece == wQueen || promo_piece == bQueen)) ||
                    (promo_char == 'r' && (promo_piece == wRook || promo_piece == bRook)) ||
                    (promo_char == 'b' && (promo_piece == wBishop || promo_piece == bBishop)) ||
                    (promo_char == 'n' && (promo_piece == wKnight || promo_piece == bKnight))) {
                    return move; // Found promotion move
                }
            } else {
                return move; // Found normal move
            }
        }
    }
    return 0; // Move not found
}

// --- Helper to print a move in UCI format ---
static void print_move_uci(int move) {
    char from_sq_str[3], to_sq_str[3];
    square_to_algebraic(MOVE_FROM(move), from_sq_str);
    square_to_algebraic(MOVE_TO(move), to_sq_str);
    printf("%s%s", from_sq_str, to_sq_str);

    int promo_piece = MOVE_PROMOTION(move);
    if (promo_piece) {
        if (promo_piece == wQueen || promo_piece == bQueen) printf("q");
        else if (promo_piece == wRook || promo_piece == bRook) printf("r");
        else if (promo_piece == wBishop || promo_piece == bBishop) printf("b");
        else if (promo_piece == wKnight || promo_piece == bKnight) printf("n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        return 1; // Invalid arguments
    }

    int depth = atoi(argv[1]);
    const char *fen = argv[2];

    init_all_piece_tables();
    init_hash_keys();

    Bitboards board;
    parse_fen(&board, fen);

    // Apply moves if they are provided
    if (argc == 4) {
        char *moves_str = argv[3];
        char *move_token = strtok(moves_str, " ");
        while (move_token != NULL) {
            int move = parse_move_uci(&board, move_token);
            if (move) {
                make_move(&board, move);
            }
            move_token = strtok(NULL, " ");
        }
    }

    if (depth == 0) {
        printf("\n1\n");
        return 0;
    }

    // --- Generate and print perft results in the required format ---
    moveList list;
    generate_all_moves(&board, board.side, &list);

    U64 total_nodes = 0;

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        make_move(&board, move);

        int king_sq = __builtin_ctzll(board.kings[board.side ^ 1]);
        if (!is_square_attacked(&board, king_sq, board.side)) {
            U64 nodes = perft(&board, depth - 1);
            total_nodes += nodes;
            print_move_uci(move);
            printf(" %llu\n", nodes);
        }
        
        unmake_move(&board);
    }

    // Required format: blank line, then total
    printf("\n%llu\n", total_nodes);

    return 0;
}
