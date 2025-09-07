//main.c

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "definitions.h"
#include "hashkeys.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"

#define DEFAULT_HASH_SIZE 64



int main() {
    // Initialize all the attack tables and hash keys first
    init_all_piece_tables();
    init_hash_keys();

    
    Bitboards board;
    SearchInfo info;
    initialize_bitboards(&board); // Initialize to starting position
    init_hashtable(DEFAULT_HASH_SIZE);
    // iwroteprintbitboard(board.queens[BLACK] | board.queens[WHITE]);

    // Start listening for UCI commands
    uci_loop(&board, &info);

    return 0;
}
