#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "aLaw.h"

#define ENCODED_LUT_SIZE 4096
#define DECODED_LUT_SIZE 256

int main(void) {
    uint8_t encoded_lut[ENCODED_LUT_SIZE];
    int16_t decoded_lut[DECODED_LUT_SIZE];

    // Correct: cover all possible 12-bit magnitudes (0–4095)
    for (int i = 0; i < ENCODED_LUT_SIZE; i++) {
        int16_t sample = i << 4;  // Reverse of magnitude >> 4
        encoded_lut[i] = a_law_encode(sample);
    }

    for (uint16_t i = 0; i < DECODED_LUT_SIZE; i++) {
        decoded_lut[i] = a_law_decode((uint8_t)i);
    }

    FILE *enc_fp = fopen("encoded_lut.mem", "w");
    for (int i = 0; i < ENCODED_LUT_SIZE; i++) {
        fprintf(enc_fp, "%02X\n", encoded_lut[i]);
    }
    fclose(enc_fp);

    FILE *dec_fp = fopen("decoded_lut.mem", "w");
    for (int i = 0; i < DECODED_LUT_SIZE; i++) {
        fprintf(dec_fp, "%04X\n", (uint16_t)decoded_lut[i] & 0xFFFF);
    }
    fclose(dec_fp);

    return 0;
}
