//main.c

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bitboard.h"
#include "definitions.h"
#include "movegen.h"


// Helper function to compare strings for assertions
void assert_string_equals(const char* actual, const char* expected) {
    if (strcmp(actual, expected) != 0) {
        printf("Assertion Failed: Expected \"%s\", but got \"%s\"\n", expected, actual);
        assert(0);
    }
}

void test_special_pawn_moves() {
    printf("--- Testing Special Pawn Moves ---\n");

    // Test Case 1: Promotion
    Bitboards bb1 = {0};
    setbit(bb1.pawns[WHITE], A7);
    setbit(bb1.rooks[BLACK], B8); // Blocker and capture target
    bb1.occupied[WHITE] = bb1.pawns[WHITE];
    bb1.occupied[BLACK] = bb1.rooks[BLACK];
    bb1.all_pieces = bb1.occupied[WHITE] | bb1.occupied[BLACK];
    
    moveList list1;
    generate_all_moves(&bb1, WHITE, &list1);
    // Should find 4 promotion pushes (a8=Q,R,B,N) and 4 promotion captures (b8=Q,R,B,N)
    assert(list1.count == 8);
    printf("✅ Promotion generation test passed!\n");

    // Test Case 2: En Passant
    Bitboards bb2 = {0};
    setbit(bb2.pawns[WHITE], E5);
    setbit(bb2.pawns[BLACK], D5); // The pawn that just moved two squares
    bb2.enPas = D6; // The en passant target square
    bb2.occupied[WHITE] = bb2.pawns[WHITE];
    bb2.occupied[BLACK] = bb2.pawns[BLACK];
    bb2.all_pieces = bb2.occupied[WHITE] | bb2.occupied[BLACK];

    moveList list2;
    generate_all_moves(&bb2, WHITE, &list2);
    
    int found_ep = 0;
    for (int i = 0; i < list2.count; i++) {
        if (list2.moves[i] & MOVE_IS_ENPASSANT) {
            assert(MOVE_FROM(list2.moves[i]) == E5);
            assert(MOVE_TO(list2.moves[i]) == D6);
            found_ep = 1;
        }
    }
    assert(found_ep == 1);
    printf("✅ En Passant generation test passed!\n");
}


void run_tests() {
    printf("--- Running All Tests ---\n\n");

    // 1. Test Algebraic to Square Conversion
    printf("Testing algebraic_to_square()...\n");
    assert(algebraic_to_square("a1") == A1);
    assert(algebraic_to_square("h1") == H1);
    assert(algebraic_to_square("a8") == A8);
    assert(algebraic_to_square("h8") == H8);
    assert(algebraic_to_square("e4") == E4);
    printf("✅ Passed!\n\n");

    // 2. Test Square to Algebraic Conversion
    printf("Testing square_to_algebraic()...\n");
    char notation[3];
    square_to_algebraic(A1, notation);
    assert_string_equals(notation, "a1");
    square_to_algebraic(H8, notation);
    assert_string_equals(notation, "h8");
    square_to_algebraic(E4, notation);
    assert_string_equals(notation, "e4");
    printf("✅ Passed!\n\n");

    // 3. Test Initial Board Setup
    printf("Testing initial board setup...\n");
    Bitboards bb;
    initialize_bitboards(&bb);
    
    // Check a few key pieces for both colors
    assert(isset(bb.pawns[WHITE], A2));
    assert(isset(bb.pawns[WHITE], H2));
    assert(isset(bb.knights[WHITE], B1));
    assert(isset(bb.bishops[WHITE], F1));
    assert(isset(bb.rooks[WHITE], A1));
    assert(isset(bb.queens[WHITE], D1));
    assert(isset(bb.kings[WHITE], E1));
    
    assert(isset(bb.pawns[BLACK], A7));
    assert(isset(bb.pawns[BLACK], H7));
    assert(isset(bb.knights[BLACK], G8));
    assert(isset(bb.bishops[BLACK], C8));
    assert(isset(bb.rooks[BLACK], H8));
    assert(isset(bb.queens[BLACK], D8));
    assert(isset(bb.kings[BLACK], E8));

    // Check that a middle square is empty
    assert(!isset(bb.all_pieces, E4));
    
    printf("White Pawns:\n");
    print_bitboard(bb.pawns[WHITE]);
    printf("Black Pawns:\n");
    print_bitboard(bb.pawns[BLACK]);
    printf("All Pieces:\n");
    print_bitboard(bb.all_pieces);
    printf("✅ Passed!\n\n");

    // 4. Test Bit Manipulation
    printf("Testing bit manipulation functions...\n");
    U64 test_board = 0ULL;
    setbit(test_board, D4); // 27
    setbit(test_board, B6); // 41
    setbit(test_board, G1); // 6

    assert(cntbits(test_board) == 3);

    int lsb_index;
    // Test popabit on a multi-bit board
    U64 temp_board = test_board; // Use a copy for popabit testing
    popabit(&temp_board, &lsb_index);
    assert(lsb_index == G1); // LSB should be 6 (g1)
    assert(cntbits(temp_board) == 2);

    popabit(&temp_board, &lsb_index);
    assert(lsb_index == D4); // Next LSB should be 27 (d4)
    assert(cntbits(temp_board) == 1);
    
    // Test popabit on a single-bit board
    popabit(&temp_board, &lsb_index);
    assert(lsb_index == B6); // Final bit should be 41 (b6)
    assert(cntbits(temp_board) == 0);
    assert(temp_board == 0ULL);

    // Test popabit on an empty board
    popabit(&temp_board, &lsb_index);
    assert(lsb_index == -1); // Should return -1 for an empty board

    printf("✅ popabit() and cntbits() passed!\n");

    // Test popbits (which gets all set bits at once)
    int indices[64];
    int count = popbits(test_board, indices); // Use original test_board
    assert(count == 3);
    assert(indices[0] == G1); // 6
    assert(indices[1] == D4); // 27
    assert(indices[2] == B6); // 41
    printf("✅ popbits() passed!\n\n");


    
    
    printf("--- All Tests Completed Successfully ---\n");
}

// In main.c, add this new test function

void test_knight_attack_generation() {
    printf("Testing knight attack generation...\n");

    // First, you MUST initialize the lookup table
    init_knight_attacks();

    // Test Case 1: Knight on A1 (a corner)
    U64 attacks_from_a1 = knight_attack_table[A1];
    U64 expected_a1 = (1ULL << C2) | (1ULL << B3); // Should attack only 2 squares
    assert(attacks_from_a1 == expected_a1);

    // Test Case 2: Knight on E4 (the center)
    U64 attacks_from_e4 = knight_attack_table[E4];
    U64 expected_e4 = (1ULL << C3) | (1ULL << G3) | (1ULL << C5) | (1ULL << G5) |
                      (1ULL << D2) | (1ULL << F2) | (1ULL << D6) | (1ULL << F6);
    assert(attacks_from_e4 == expected_e4);
    
    // Your original test case, but now corrected and complete
    // Test Case 3: Knight on B1 (an edge)
    U64 attacks_from_b1 = knight_attack_table[B1];
    // A knight on B1 attacks A3, C3, and D2.
    U64 expected_b1 = (1ULL << A3) | (1ULL << C3) | (1ULL << D2);
    assert(attacks_from_b1 == expected_b1);


    printf("✅ Knight attacks passed!\n\n");
}



int main() {
    run_tests();
    init_knight_attacks();
    init_king_attacks();
    init_sliding_rays();
    init_pawn_attacks(); // <-- Add new init

    printf("Running All Tests");
    test_special_pawn_moves();
    return 0;
}
