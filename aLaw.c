#include <stdint.h>

#include "aLaw.h"


// Get the sign for signed-magnitude representation
int get_sign(int16_t sample) {
    return (sample < 0) ? 0 : 1; // 0 for negative, 1 for positive
}

// Get the 12-bit magnitude
uint16_t get_magnitude(int16_t sample) {
    uint16_t magnitude = (sample < 0) ? -sample : sample;
    magnitude = magnitude >> MAGNITUDE_SHIFT;
    return magnitude;
}

// Get the chord for a 12-bit sample
int get_chord(uint16_t magnitude) {
    return 27 - __builtin_clz(magnitude);
}

// Get the step (the 4 bits after MSB)
int get_step(int magnitude, int chord) {
    return (magnitude >> chord) & 0x0F;
}

// Compress a 16-bit signed integer sample to 8-bit A-law format
uint8_t a_law_encode(int16_t sample){
    int sign = get_sign(sample);
    uint16_t magnitude = get_magnitude(sample);

    if (magnitude < 0b10000) {
        return (((magnitude >> 1) | (sign << 7)) ^ INVERSION_MASK);
    }

    int chord = get_chord(magnitude);
    int step = get_step(magnitude, chord);

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
        temp_sample = 0x01 | step << 1; // Small magnitude case
        if (!sign) {
            temp_sample = -temp_sample; // Apply sign
        }
        return temp_sample << MAGNITUDE_SHIFT;
    }
    temp_sample = 0x21 | step << 1;
    temp_sample <<= chord-1;
    if (!sign) {
        temp_sample = -temp_sample;
    }
    temp_sample =  temp_sample << MAGNITUDE_SHIFT;
    return temp_sample;
}
