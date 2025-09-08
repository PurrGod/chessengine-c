#include "hashkeys.h"
#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

TT_entry *transposition_table = NULL;
size_t numEntries = 0;

void clear_TT();

void init_hashtable(int megabytes) {
    size_t bytes = (size_t)megabytes * 1024 * 1024;
    numEntries = bytes / sizeof(TT_entry);
   
    if (transposition_table != NULL){free(transposition_table);}

    transposition_table = (TT_entry *) malloc (numEntries * sizeof(TT_entry));

    // Check if the allocation was successful
    if (transposition_table == NULL) {
        printf("info string Error: Failed to allocate transposition table.\n");
        // Exit or handle the error gracefully
        exit(1); 
    } else {
        printf("info string Transposition table initialized with %ld entries.\n", numEntries);
        clear_TT();
    }

}

void clear_TT(){
    if (transposition_table != NULL){memset(transposition_table, 0, numEntries * sizeof(TT_entry));}
}

TT_entry *probe_TT(U64 posKey) {
    size_t index = posKey % numEntries;
    TT_entry * entree = &transposition_table[index];

    if (posKey == entree->posKey) {
        return entree;
    }
    
    return NULL;
}

void store_in_tt(U64 posKey, int depth, int beta, int move, int flag) {
    size_t index = posKey % numEntries;
    TT_entry *entry = &transposition_table[index];

    entry->b_move = move;
    entry->depth = (char)depth;
    entry->posKey = posKey;
    entry->score = beta;
    entry->pv_flag = (char)flag;
}