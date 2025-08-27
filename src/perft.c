#include "perft.h"
#include "definitions.h"
#include "movegen.h"
#include "attack.h"
#include "make_moves.h"


// recursively explores all the legal moves for given depth
// returns the number of nodes possible for depth which is well established
U64 perft(Bitboards * bb, int depth) {
    // initialize nodes and moveLisst
    U64 nodes = 0;
    moveList list;

    if (depth == 0) {
        return 1ULL;
    }

    // generate the moves
    generate_all_moves(bb, bb->side, &list);
    
    // iterate through the gen pseudolegal move list
    for (int i = 0; i < list.count; i++) {
        // 1. Make move
        // 2. Check legality (Is the king under attack? If yes discard, else keep exploring)
        // 3. Recursive call perft with one less depth
        // 4. unmake move
        make_move(bb, list.moves[i]);
        int king_square = __builtin_ctzll(bb->kings[!bb->side]);
        if (!is_square_attacked(bb, king_square, bb->side)){
            nodes += perft(bb, depth - 1);
        }

        unmake_move(bb);
    }

    return nodes;

}