#include <stdio.h>
#include <stdint.h>

#include "aLaw.h"


// Print a 16-bit value in signed 2's complement binary format
const char* get_bin_str(uint16_t value, int num_bits) {
    static char binstr[17]; // 16 bits + null terminator
    if (num_bits > 16) num_bits = 16;
    if (num_bits < 1) num_bits = 1;
    for (int i = num_bits - 1; i >= 0; --i) {
        binstr[num_bits - 1 - i] = (value & (1 << i)) ? '1' : '0';
    }
    binstr[num_bits] = '\0';
    return binstr;
}

// Compress a 16-bit signed integer sample to 8-bit A-law format
uint8_t a_law_compression(int16_t sample){
    int sign, magnitude;

    if (sample < 0) {
        sign = 1;
        magnitude = -sample; // Make it positive for processing
    } else {
        sign = 0;
        magnitude = sample;
    }

    // Truncate the sample to 12 bits as per A-law compression
    if (magnitude > 0xFFF) {
        magnitude = 0xFFF; // Cap to maximum 12-bit value
    }

    // Find the MSB
    int chord = 0;
    for (int i = 11; i >= 5; i--) {
        if (magnitude & (1 << i)) {
            chord = i - 5;
            break;
        }
    }

    //  Extract 4 step bits after MSB
    int step = (magnitude >> (chord + 1)) & 0xF;

    // Assemble A-law codeword (sign 1-bit | chord 3-bits | step 4-bits)
    uint8_t codeword = (sign << 7) | (chord << 4) | step;

    // Invert the codeword to match A-law encoding
    codeword ^= 0b01010101;

    return codeword;
}
