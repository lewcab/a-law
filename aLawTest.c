#include <stdio.h>
#include <stdint.h>

#include "aLaw.h" // Include the header file for aLaw.c

#define NUM_TESTS 2


int check_outcome(int test_num, int16_t sample, uint8_t result, uint8_t expected) {
    printf("Test %d:\n", test_num);
    printf("  Sample:     %d (0b%s)\n", sample, get_bin_str(sample, 16));
    printf("  Compressed: %d (0b%s)\n", result, get_bin_str(result, 8));
    printf("  Expected:   %d (0b%s)\n", expected, get_bin_str(expected, 8));
    if (result == expected) {
        printf("  (%d) PASSED\n\n", test_num);
        return 1;
    }
    printf("  (%d) FAILED\n\n", test_num);
    return 0;
}


void test_a_law_compression() {
    const int16_t samples[NUM_TESTS] = {
        0b000010110101,
        0b011101110110,
    };
    const uint8_t expecteds[NUM_TESTS] = {
        0b10110110^INVERSION_MASK,
        0b11101101^INVERSION_MASK,
    };

    int passed = 0;
    int sample, result, expected;
    for (int i = 0; i < NUM_TESTS; i++) {
        sample = samples[i];
        expected = expecteds[i];
        result = a_law_encode(sample);
        passed += check_outcome(i + 1, sample, result, expected);
    }

    // Add more test cases as needed
    printf("%d tests for a_law_compression passed.\n", passed);
}


int main() {
    test_a_law_compression();
    return 0;
}