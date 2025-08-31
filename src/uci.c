// src/uci.c

#include "definitions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "uci.h"
#include "bitboard.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"
#include "search.h" 

// --- Helper Function to parse a UCI move string ---
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
                if (strlen(uci_move_str) < 5) continue;
                char promo_char = (char)tolower(uci_move_str[4]);
                if ((promo_char == 'q' && (promo_piece == wQueen || promo_piece == bQueen)) ||
                    (promo_char == 'r' && (promo_piece == wRook || promo_piece == bRook)) ||
                    (promo_char == 'b' && (promo_piece == wBishop || promo_piece == bBishop)) ||
                    (promo_char == 'n' && (promo_piece == wKnight || promo_piece == bKnight))) {
                    return move;
                }
            } else {
                return move;
            }
        }
    }
    return 0;
}

// --- Command Handler for "go" ---
// This is now much more complex to handle time controls.
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

    // This is the new call to the main search driver
    search_position(board, info);
}

// --- Command Handler for "position" ---
static void parse_position(Bitboards *board, char *input) {
    char *fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    char *token;

    token = strtok(input, " ");
    if (strcmp(token, "startpos") == 0) {
        token = strtok(NULL, " ");
    } else if (strcmp(token, "fen") == 0) {
        char fen_buffer[200] = {0};
        token = strtok(NULL, " ");
        while (token != NULL && strcmp(token, "moves") != 0) {
            strcat(fen_buffer, token);
            strcat(fen_buffer, " ");
            token = strtok(NULL, " ");
        }
        fen_string = fen_buffer;
    }

    parse_fen(board, fen_string);

    if (token != NULL && strcmp(token, "moves") == 0) {
        token = strtok(NULL, " ");
        while (token != NULL) {
            int move = parse_move_uci(board, token);
            if (move) {
                make_move(board, move);
            }
            token = strtok(NULL, " ");
        }
    }
}

// --- Main UCI Loop ---
void uci_loop(Bitboards *board, SearchInfo *info) {
    char input[4096];
    setbuf(stdout, NULL);

    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = 0;

        if (strncmp(input, "uci", 3) == 0) {
            printf("id name PurrfectEngine\n");
            printf("id author Purrgod\n");
            printf("uciok\n");
        } else if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(input, "position", 8) == 0) {
            parse_position(board, input + 9);
        } else if (strncmp(input, "go", 2) == 0) {
            parse_go(board, input + 3, info);
        } else if (strncmp(input, "quit", 4) == 0) {
            info->stopped = 1;
            break;
        }
    }
}
