#include <stdio.h>
#include <stdint.h>

#include "aLaw.h" // Include the header file for aLaw.c


int check_outcome(int test_num, int16_t sample, uint8_t result, uint8_t expected) {
    printf("Test %d:\n", test_num);
    printf("  Sample:     0b%s\n", get_bin_str(sample, 16));
    printf("  Compressed: 0b%s\n", get_bin_str(result, 8));
    printf("  Expected:   0b%s\n", get_bin_str(expected, 8));
    if (result == expected) {
        printf("  (%d) PASSED\n\n", test_num);
        return 1;
    }
    printf("  (%d) FAILED\n\n", test_num);
    return 0;
}


void test_a_law_compression() {
    int passed = 0;
    // Test case 1
    int16_t sample1 = 0b00000010110101;
    uint8_t result1 = a_law_compression(sample1);
    uint8_t expected1 = 0b10100110;
    passed += check_outcome(1, sample1, result1, expected1);

    // Test case 2
    int16_t sample2 = 0b10011101110110;
    uint8_t result2 = a_law_compression(sample2);
    uint8_t expected2 = 0b01011101;
    passed += check_outcome(2, sample2, result2, expected2);

    // Add more test cases as needed
    printf("%d tests for a_law_compression passed.\n", passed);
}


int main() {
    test_a_law_compression();
    return 0;
}