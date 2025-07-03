#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "aLaw.h" // Include the header file for aLaw.c

void test_a_law_compression() {
    // Test case 1
    int16_t sample1 = 0b00000010110101;
    uint8_t expected1 = 0b10100110;
    uint8_t result1 = a_law_compression(sample1);
    printf("Test 1:\n");
    printf("\tSample:           %s\n", get_bin_str(sample1, 16));
    printf("\tCompressed A-law: %s\n", get_bin_str(result1, 8));
    printf("\tExpected A-law:   %s\n", get_bin_str(expected1, 8));
    assert(sample1 == expected1);

    // Test case 2
    int16_t sample2 = 0b10011101110110;
    uint8_t expected2 = 0b01011101;
    uint8_t result2 = a_law_compression(sample2);
    printf("Sample: %d, Compressed A-law: %d (expected: %d)\n", sample1, result1, expected1);
    assert(result2 == expected2);

    // Add more test cases as needed
    printf("All tests for a_law_compression passed.\n");
}

int main() {
    test_a_law_compression();
    return 0;
}