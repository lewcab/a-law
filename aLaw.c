#include <stdint.h>
#include <arm_neon.h>

#include "aLaw.h"


// Get the sign for signed-magnitude representation
uint16x8_t get_sign_neon(int16x8_t samples) {
    return vcgtzq_s16(samples);
}

// Get the 12-bit magnitude
int16x8_t get_magnitude_neon(int16x8_t samples) {
    int16x8_t abs_samples = vqabsq_s16(samples);
    return vshrq_n_s16(abs_samples, MAGNITUDE_SHIFT);
}

// Get the chord for a 12-bit sample
int16x8_t get_chord_neon(uint16x8_t magnitudes) {
    return vsubq_s16(vdupq_n_s16(27), vclzq_s16(magnitudes));
}

// Get the step (the 4 bits after MSB)
int16x8_t get_step_neon(int16x8_t magnitudes, int16x8_t chords) {
    // flip chord so left shift by chord shift to the right
    int16x8_t chords_neg = vnegq_s16(chords);
    int16x8_t shifted_magnitude = vshlq_s16(magnitudes, chords_neg);
    return vandq_s16(shifted_magnitude, vdupq_n_s16(0x0F));
}

// Assemble the A-law codeword
uint8x8_t assemble_codeword_neon(int16x8_t signs, int16x8_t chords, int16x8_t steps) {
    uint8x8_t signs_u8 = vshl_n_u8(vreinterpret_u8_s16(signs), 7);
    uint8x8_t chords_u8 = vshl_n_u8(vreinterpret_u8_s16(chords), 4);
    uint8x8_t steps_u8 = vreinterpret_u8_s16(steps);

    return vorr_u8(signs_u8, vorr_u8(chords_u8, steps_u8));
}

/**
 * Compress 16-bit signed integer samples to 8-bit A-law codewords using SIMD.
 * @param samples input array of 16-bit signed integers
 * @param codewords output array of 8-bit A-law encoded values
 * @param num_samples number of samples to encode
 */
void a_law_encode_neon(int16_t *samples, int8_t *codewords, int num_samples) {
    int num_16x8 = (num_samples + 7) / 8;
    int idx = 0;
    for (int i = 0; i < num_16x8; i++) {
        idx = i * 8;
        // Load 8 signed 16-bit samples into a NEON vector
        int16x8_t vec_samples = vld1q_s16(&samples[idx]);

        // Get the sign (0 for negative, 1 for positive)
        int16x8_t vec_signs = get_sign_neon(vec_samples);

        // Get the magnitude (absolute value)
        int16x8_t vec_magnitudes = get_magnitude_neon(vec_samples);

        // Get the chord
        int16x8_t vec_chords = get_chord_neon(vec_magnitudes);

        // Get the step
        int16x8_t step = get_step_neon(vec_magnitudes, vec_chords);

        // Assemble the A-law codeword
        uint8x8_t codeword = assemble_codeword_neon(vec_signs, vec_chords, step);

        // Invert the codeword
        codeword = veor_u8(codeword, vdup_n_u8(INVERSION_MASK));

        // Store the result
        vst1_u8(&codewords[i], codeword);
    }

}
