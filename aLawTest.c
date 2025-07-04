#include <stdio.h>
#include <stdint.h>

#include "aLaw.h" // Include the header file for aLaw.c

#define NUM_TESTS 5


int check_outcome(int test_num, int16_t sample, uint8_t enc_res, uint8_t enc_exp, int16_t dec_res, int16_t dec_exp) {
    printf("Test %d:\n", test_num);
    printf("  Sample:   %4d (0b%s)\n", sample, get_bin_str(sample, 16));
    printf("  Encoding:\n");
    printf("    Result: %4d (0b%s)\n", enc_res, get_bin_str(enc_res, 8));
    printf("    Expect: %4d (0b%s)\n", enc_exp, get_bin_str(enc_exp, 8));
    printf("    %s\n", enc_res == enc_exp ? "o- PASSED -o" : "x- FAILED -x");
    printf("  Decoding:\n");
    printf("    Result: %4d (0b%s)\n", dec_res, get_bin_str(dec_res, 16));
    printf("    Expect: %4d (0b%s)\n", dec_exp, get_bin_str(dec_exp, 16));
    printf("    %s\n", dec_res == dec_exp ? "o- PASSED -o" : "x- FAILED -x");
    printf("\n");
    return enc_res == enc_exp && dec_res == dec_exp;
}


void test_a_law() {
    const int16_t samples[NUM_TESTS] = {
        //s............
        0b0000000000011,
        0b0000000011111,
        0b0000000111111,
        0b0000010110101,
        0b0011101110110,
    };
    const uint8_t encoded[NUM_TESTS] = {
        //sxxxabcd
        0b10000001^INVERSION_MASK,
        0b10001111^INVERSION_MASK,
        0b10011111^INVERSION_MASK,
        0b10110110^INVERSION_MASK,
        0b11101101^INVERSION_MASK,
    };
    const int16_t decoded[NUM_TESTS] = {
        //s............
        0b0000000000011,
        0b0000000011111,
        0b0000000111111,
        0b0000010110100,
        0b0011101100000,
    };

    int passed = 0;
    int16_t sample, dec_exp, dec_res;
    uint8_t enc_exp, enc_res;
    for (int i = 0; i < NUM_TESTS; i++) {
        sample = samples[i];
        enc_exp = encoded[i];
        enc_res = a_law_encode(sample);
        dec_exp = decoded[i];
        dec_res = a_law_decode(enc_exp);
        passed += check_outcome(i + 1, sample, enc_res, enc_exp, dec_res, dec_exp);
    }

    printf("%d/%d tests for a_law_compression passed.\n", passed, NUM_TESTS);
}


int main() {
    test_a_law();
    return 0;
}