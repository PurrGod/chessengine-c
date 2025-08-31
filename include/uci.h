// include/uci.h

#ifndef UCI_H
#define UCI_H

#include "definitions.h"

// Function to start the main UCI communication loop
void uci_loop(Bitboards *board, SearchInfo * SearchInfo);
void print_move_uci(int move);

#endif // UCI_H
