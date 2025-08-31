// src/uci.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "definitions.h"
#include "uci.h"
#include "bitboard.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "search.h" 

// --- Helper Function to print a move in UCI format ---
// e.g., "e2e4", "a7a8q"
void print_move_uci(int move) {
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

// --- Helper Function to parse a UCI move string ---
// Finds the corresponding move integer for a string like "e2e4"
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
                // Check if the move string includes a promotion character
                if (strlen(uci_move_str) < 5) continue;
                char promo_char = (char)tolower(uci_move_str[4]);
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


// --- Command Handler for "go" ---
static void parse_go(Bitboards *board, char *input, SearchInfo *info) {
    int depth = -1, movestogo = 30, movetime = -1;
    int time = -1, inc = 0;
    char *ptr = NULL;
    info->timeset = 0;

    if ((ptr = strstr(input, "depth"))) {
        depth = atoi(ptr + 6);
    }
    if ((ptr = strstr(input, "movetime"))) {
        movetime = atoi(ptr + 9);
    }
    if ((ptr = strstr(input, "wtime")) && board->side == WHITE) {
        time = atoi(ptr + 6);
    }
    if ((ptr = strstr(input, "btime")) && board->side == BLACK) {
        time = atoi(ptr + 6);
    }
    if ((ptr = strstr(input, "winc")) && board->side == WHITE) {
        inc = atoi(ptr + 5);
    }
    if ((ptr = strstr(input, "binc")) && board->side == BLACK) {
        inc = atoi(ptr + 5);
    }

    info->starttime = get_time_ms();
    info->depth = depth;

    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    if (time != -1) {
        info->timeset = 1;
        time /= movestogo;
        time -= 50; // A small buffer
        info->stoptime = info->starttime + time + inc;
    }

    if (depth == -1) {
        info->depth = MAX_DEPTH;
    }
    
    printf("info time:%d start:%ld stop:%ld depth:%d timeset:%d\n",
           time, info->starttime, info->stoptime, info->depth, info->timeset);

    search_position(board, info);
}

// --- Command Handler for "position" ---
static void parse_position(Bitboards *board, char *input) {
    // Shift pointer past "position "
    input += 9;
    char *current_char = input;
    
    if (strncmp(input, "startpos", 8) == 0) {
        parse_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else {
        current_char = strstr(input, "fen");
        if (current_char == NULL) {
            parse_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        } else {
            current_char += 4; // Shift past "fen "
            parse_fen(board, current_char);
        }
    }
    
    current_char = strstr(input, "moves");
    if (current_char != NULL) {
        current_char += 6; // Shift past "moves "
        while (*current_char) {
            int move = parse_move_uci(board, current_char);
            if (move == 0) {
                break;
            }
            make_move(board, move);
            // Move pointer to the next move
            while (*current_char && *current_char != ' ') {
                current_char++;
            }
            // Skip the space
            if (*current_char) {
                current_char++;
            }
        }
    }
}

// --- Main UCI Loop ---
void uci_loop(Bitboards *board, SearchInfo *info) {
    char input[4096];
    setbuf(stdout, NULL);

    printf("id name PurrfectEngine\n");
    printf("id author Purrgod\n");
    printf("uciok\n");

    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;

        if (input[0] == '\n') {
            continue;
        }
        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(input, "position", 8) == 0) {
            parse_position(board, input);
        } else if (strncmp(input, "ucinewgame", 10) == 0) {
            parse_position(board, "position startpos");
        } else if (strncmp(input, "go", 2) == 0) {
            parse_go(board, input, info);
        } else if (strncmp(input, "quit", 4) == 0) {
            info->stopped = 1;
            break;
        } else if (strncmp(input, "uci", 3) == 0) {
            printf("id name PurrfectEngine\n");
            printf("id author Purrgod\n");
            printf("uciok\n");
        }
    }
}

