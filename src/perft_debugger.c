// src/perft_debugger.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "definitions.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "hashkeys.h"
#include "attack.h"
#include "make_moves.h"

#define MAX_FEN_LENGTH 200

// Helper to print a move in algebraic notation
void print_move_algebraic(int move) {
    char from_sq_str[3];
    char to_sq_str[3];
    square_to_algebraic(MOVE_FROM(move), from_sq_str);
    square_to_algebraic(MOVE_TO(move), to_sq_str);
    printf("%s%s", from_sq_str, to_sq_str);

    // Handle promotions
    int promo_piece = MOVE_PROMOTION(move);
    if (promo_piece) {
        switch(promo_piece) {
            case wQueen: case bQueen: printf("q"); break;
            case wRook:  case bRook:  printf("r"); break;
            case wBishop:case bBishop:printf("b"); break;
            case wKnight:case bKnight:printf("n"); break;
        }
    }
}

int main(int argc, char *argv[]) {
    // --- Argument Validation ---
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <fen_file> <depth>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int depth = atoi(argv[2]);

    if (depth < 1) {
        fprintf(stderr, "Error: Depth must be at least 1.\n");
        return 1;
    }

    // --- File Reading ---
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    char fen[MAX_FEN_LENGTH];
    if (fgets(fen, sizeof(fen), file) == NULL) {
        fprintf(stderr, "Error reading FEN from file.\n");
        fclose(file);
        return 1;
    }
    fclose(file);
    fen[strcspn(fen, "\n")] = 0; // Remove newline

    // --- Engine Initialization ---
    init_all_piece_tables();
    init_hash_keys();

    Bitboards board;
    parse_fen(&board, fen);

    printf("Debugging Perft for FEN: %s\n", fen);
    printf("Depth: %d\n\n", depth);
    print_board(&board);

    // --- Generate Root Moves ---
    moveList list;
    generate_all_moves(&board, board.side, &list);

    U64 total_nodes = 0;
    printf("Move   | Nodes\n");
    printf("-------+----------\n");

    // --- Iterate and Perft for each move ---
    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];

        make_move(&board, move);
        // Check if the move was legal (king is not in check after move)
        int king_square = __builtin_ctzll(board.kings[board.side ^ 1]);
        if (!is_square_attacked(&board, king_square, board.side)) {
            U64 nodes_for_move = (depth > 1) ? perft(&board, depth - 1) : 1;
            total_nodes += nodes_for_move;

            print_move_algebraic(move);
            printf("\t | %llu\n", nodes_for_move);
        }
        unmake_move(&board);
    }

    printf("-------+----------\n");
    printf("Total\t | %llu\n", total_nodes);

    return 0;
}
