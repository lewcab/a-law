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
uint8_t a_law_encode(int16_t sample){
    int sign, magnitude;

    if (sample < 0) {
        sign = 0;
        magnitude = -sample; // Make it positive for processing
    } else {
        sign = 1;
        magnitude = sample;
    }

    // Truncate the sample to 12 bits as per A-law compression
    if (magnitude > 0xFFF) {
        magnitude = 0xFFF; // Cap to maximum 12-bit value
    }
    if (magnitude < 0b10000) {
        return (((magnitude >> 1) | (sign << 7)) ^ INVERSION_MASK);
    }

    // Find the MSB
    // TODO: optimize... CLZ instead of loop?
    int chord = 0;
    for (int i = 11; i >= 4; i--) {
        if (magnitude & (1 << i)) {
            chord = i - 4;
            break;
        }
    }

    //  Extract 4 step bits after MSB
    int step = (magnitude >> chord) & 0x0F;

    // Assemble A-law codeword (sign 1-bit | chord 3-bits | step 4-bits)
    uint8_t codeword = (sign << 7) | (chord << 4) | step;

    // Invert the codeword to match A-law encoding
    codeword ^= INVERSION_MASK;

    return codeword;
}

// Decompress an 8-bit A-law codeword to a 16-bit signed integer sample
int16_t a_law_decode(uint8_t codeword) {
    // Invert the codeword to get the original A-law codeword
    uint8_t temp_codeword = codeword ^ INVERSION_MASK;

    // Extract sign, chord, and step bits
    int sign = (temp_codeword >> 7) & 0x01;
    int chord = (temp_codeword >> 4) & 0x07;
    int step = temp_codeword & 0x0F; // abcd

    // Reconstruct into Signed-Magnitude representation
    int16_t temp_sample;
    if (chord == 0) {
        // 0b0000000abcd1
        temp_sample = 0x01 | step << 1; // Small magnitude case
        if (!sign) {
            temp_sample = -temp_sample; // Apply sign
        }
        return temp_sample;
    }
    // 0b...1abcd1...
    temp_sample = 0x21 | step << 1;
    temp_sample <<= chord-1;
    if (!sign) {
        temp_sample = -temp_sample; // Apply sign
    }
    return temp_sample;
}
