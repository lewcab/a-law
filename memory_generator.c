#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "aLaw.h"

#define ENCODED_LUT_SIZE 65536
#define DECODED_LUT_SIZE 256

int main(void) {
    uint8_t encoded_lut[ENCODED_LUT_SIZE];  // Input: int16_t (-32768 to 32767) → Output: uint8_t
    int16_t decoded_lut[DECODED_LUT_SIZE];  // Input: uint8_t (0 to 255)       → Output: int16_t

    // Fill encoded LUT: full range of signed 16-bit input samples
    for (int32_t i = -32768; i <= 32767; i++) {
        encoded_lut[i + 32768] = a_law_encode((int16_t)i);
    }

    // Fill decoded LUT: all 256 possible 8-bit A-law values
    for (uint16_t i = 0; i < DECODED_LUT_SIZE; i++) {
        decoded_lut[i] = a_law_decode((uint8_t)i);
    }

    // Optional: Write to binary file or .mem format
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
