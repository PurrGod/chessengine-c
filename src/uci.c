// src/uci.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "definitions.h"
#include "uci.h"
#include "bitboard.h"
#include "perft.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"

// Helper to print a move in algebraic notation for debugging
static void print_move_algebraic(int move) {
    char from_sq_str[3];
    char to_sq_str[3];
    square_to_algebraic(MOVE_FROM(move), from_sq_str);
    square_to_algebraic(MOVE_TO(move), to_sq_str);
    printf("%s%s", from_sq_str, to_sq_str);

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

// Main function to handle the "go perft" command
static void parse_go(Bitboards *board, char *input) {
    int depth = 0;
    char *arg = strtok(input, " ");

    // Find the "depth" argument
    while (arg != NULL) {
        if (strcmp(arg, "depth") == 0) {
            arg = strtok(NULL, " ");
            if (arg != NULL) {
                depth = atoi(arg);
            }
            break;
        }
        arg = strtok(NULL, " ");
    }

    if (depth <= 0) {
        printf("info string Invalid depth\n");
        return;
    }

    // This is the core of the perft command for perftree
    moveList list;
    generate_all_moves(board, board->side, &list);

    printf("Perft results for depth %d:\n", depth);

    for (int i = 0; i < list.count; i++) {
        int move = list.moves[i];
        make_move(board, move);

        // Check for legality
        int king_square = __builtin_ctzll(board->kings[board->side ^ 1]);
        if (!is_square_attacked(board, king_square, board->side)) {
            U64 nodes = (depth > 1) ? perft(board, depth - 1) : 1;
            print_move_algebraic(move);
            printf(": %llu\n", nodes);
        }
        
        unmake_move(board);
    }
    // perftree requires a final "nodes" line, but we can omit it for simplicity
    // as it calculates the total itself.
}

// Function to handle the "position" command
static void parse_position(Bitboards *board, char *input) {
    char *fen_string = NULL;

    if (strncmp(input, "startpos", 8) == 0) {
        fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    } else if (strncmp(input, "fen", 3) == 0) {
        fen_string = input + 4; // Point to the start of the actual FEN
    }

    if (fen_string) {
        parse_fen(board, fen_string);
    }
}

// The main loop that listens for and processes UCI commands
void uci_loop(Bitboards *board) {
    char input[2048];

    setbuf(stdout, NULL); // Ensure output is not buffered

    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0; // Remove newline

        if (strncmp(input, "uci", 3) == 0) {
            printf("id name MyChessEngine\n");
            printf("id author YourName\n");
            printf("uciok\n");
        } else if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(input, "position", 8) == 0) {
            parse_position(board, input + 9);
        } else if (strncmp(input, "go perft", 8) == 0) {
            parse_go(board, input + 3); // Pass "perft depth X"
        } else if (strncmp(input, "quit", 4) == 0) {
            break;
        }
    }
}
