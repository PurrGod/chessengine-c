// src/perft_test.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "definitions.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "hashkeys.h"

#define MAX_LINE_LENGTH 512

// Forward declaration for the parse_fen function
void parse_fen(Bitboards *bb, const char *fen);

int main(int argc, char *argv[]) {
    // --- Argument Validation ---
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <test_file.epd> <depth>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int requested_depth = atoi(argv[2]);

    if (requested_depth <= 0) {
        fprintf(stderr, "Error: Depth must be a positive integer.\n");
        return 1;
    }

    // --- File Reading ---
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // --- Engine Initialization ---
    init_all_piece_tables();
    init_hash_keys();

    char line[MAX_LINE_LENGTH];
    int position_count = 1;
    int tests_run = 0;

    printf("Running tests for Depth %d from file: %s\n", requested_depth, filename);

    // --- Loop through each line in the file ---
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) < 10) continue;

        char line_copy[MAX_LINE_LENGTH];
        strcpy(line_copy, line);

        char *fen = strtok(line_copy, ";");
        if (fen == NULL) continue;

        // Check if there is any perft data after the FEN
        char *perft_data_string = strtok(NULL, ""); // Get the rest of the string

        if (perft_data_string == NULL) {
            // --- NO PERFT DATA: Just run the test and print results ---
            tests_run++;
            printf("\n----------------------------------------------------------------------\n");
            printf("Position #%d (No comparison data): %s\n", position_count, fen);

            Bitboards board;
            parse_fen(&board, fen);

            clock_t start = clock();
            U64 engine_nodes = perft(&board, requested_depth);
            clock_t end = clock();
            double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

            print_board(&board);
            printf("  Depth %d: Engine found %llu nodes in %.4f seconds.\n",
                   requested_depth, engine_nodes, time_spent);

        } else {
            // --- PERFT DATA EXISTS: Find matching depth and compare ---
            U64 expected_nodes = 0;
            int found_depth = 0;
            char *token = strtok(perft_data_string, ";");

            while (token != NULL) {
                int depth_from_file;
                U64 nodes_from_file;
                if (sscanf(token, " D%d %llu", &depth_from_file, &nodes_from_file) == 2) {
                    if (depth_from_file == requested_depth) {
                        expected_nodes = nodes_from_file;
                        found_depth = 1;
                        break;
                    }
                }
                token = strtok(NULL, ";");
            }

            if (found_depth) {
                tests_run++;
                printf("\n----------------------------------------------------------------------\n");
                printf("Position #%d: %s\n", position_count, fen);

                Bitboards board;
                parse_fen(&board, fen);

                clock_t start = clock();
                U64 engine_nodes = perft(&board, requested_depth);
                clock_t end = clock();
                
                long long diff = (long long)engine_nodes - (long long)expected_nodes;

                printf("  Result: Engine(%12llu) | Correct(%12llu) | Diff(%10lld) | %s\n",
                       engine_nodes, expected_nodes, diff, (diff == 0) ? "✅ OK" : "❌ FAIL");
                printf("Time taken: %ld", end-start);
            }
        }
        position_count++;
    }

    printf("\n----------------------------------------------------------------------\n");
    if (tests_run == 0) {
        printf("Test complete. No positions found in the file with data for Depth %d.\n", requested_depth);
    } else {
        printf("Test complete. Ran checks for %d position(s).\n", tests_run);
    }

    fclose(file);
    return 0;
}
