#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "definitions.h"  // for algebraic conversion
#include "bitboard.h"     // for setbit, clearbit, etc.

void test_setbit_and_clearbit() {
    U64 bb = 0;
    setbit(bb, 36); // e4
    assert(bb == (1ULL << 36));

    clearbit(bb, 36);
    assert(bb == 0);

    printf("✅ setbit/clearbit passed\n");
}

void test_cntbits_and_popabit() {
    U64 bb = 0;
    setbit(bb, 12); // e2
    setbit(bb, 36); // e4
    setbit(bb, 63); // a8

    assert(cntbits(bb) == 3);

    int idx;
    popabit(&bb, &idx);
    assert(idx == 12); // LSB
    assert(cntbits(bb) == 2);

    printf("✅ cntbits/popabit passed\n");
}

void test_algebraic_conversion() {
    assert(algebraic_to_square("a8") == 63);
    assert(algebraic_to_square("h1") == 0);
    assert(algebraic_to_square("e4") == 36);

    char sq[3];
    square_to_algebraic(36, sq);
    assert(strcmp(sq, "e4") == 0);

    printf("✅ algebraic conversions passed\n");
}

void test_popbits() {
    U64 bb = 0;
    setbit(bb, 2);
    setbit(bb, 40);
    setbit(bb, 63);

    int indices[64];
    int count = popbits(bb, indices);

    assert(count == 3);
    assert(indices[0] == 2);
    assert(indices[1] == 40);
    assert(indices[2] == 63);

    printf("✅ popbits passed\n");
}


int main() {
    test_setbit_and_clearbit();
    test_cntbits_and_popabit();
    test_algebraic_conversion();
    test_popbits();
    return 0;
}
