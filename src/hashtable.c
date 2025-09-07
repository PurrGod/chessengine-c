#include "hashkeys.h"
#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

TT_entry *transposition_table = NULL;
long numEntries = 0;


void init_hashtable(int megabytes) {
    long bytes = megabytes * 1024 * 1024;
    int numEntries = bytes / sizeof(TT_entry);
   
    if (transposition_table != NULL){free(transposition_table);}

    transposition_table = (TT_entry *) malloc (numEntries * sizeof(TT_entry));

    // Check if the allocation was successful
    if (transposition_table == NULL) {
        printf("info string Error: Failed to allocate transposition table.\n");
        // Exit or handle the error gracefully
        exit(1); 
    } else {
        printf("info string Transposition table initialized with %ld entries.\n", numEntries);
        clear_transposition_table();
    }

}

void clear_TT(){
    if (transposition_table != NULL){memset(transposition_table, 0, numEntries * sizeof(TT_entry));}
}