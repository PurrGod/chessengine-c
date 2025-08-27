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




int main() {
    // Initialize all the attack tables and hash keys first
    init_all_piece_tables();
    init_hash_keys();

    Bitboards board;
    initialize_bitboards(&board); // Initialize to starting position

    // Start listening for UCI commands
    uci_loop(&board);

    return 0;
}
