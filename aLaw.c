#include <stdint.h>
#include <arm_neon.h>

#include "aLaw.h"


// Get the sign for signed-magnitude representation
uint8x8_t get_sign_neon(int16x8_t samples) {
    uint16x8_t signs = vcgeq_s16(samples, vdupq_n_s16(0));
    uint8x8_t signs_u8 = vmovn_u16(signs);
	return vand_u8(signs_u8, vdup_n_u8(1));
}

// Get the 12-bit magnitude
int16x8_t get_magnitude_neon(int16x8_t samples) {
    int16x8_t abs_samples = vabsq_s16(samples);
    return vshrq_n_s16(abs_samples, MAGNITUDE_SHIFT);
}

// Get the chord for a 12-bit sample
int16x8_t get_chord_neon(int16x8_t magnitudes) {
    int16x8_t clz_result = vclzq_s16(magnitudes);
    return vsubq_s16(vdupq_n_s16(11), clz_result);
}

// Get the step (the 4 bits after MSB)
uint8x8_t get_step_neon(int16x8_t magnitudes, int16x8_t chords) {
    int16x8_t chords_neg = vnegq_s16(chords);
    int16x8_t shifted_magnitude = vshlq_s16(magnitudes, chords_neg);
    int16x8_t steps = vandq_s16(shifted_magnitude, vdupq_n_s16(0x0F));
    return vreinterpret_u8_s8(vmovn_s16(steps));
}

// Assemble the A-law codeword
uint8x8_t assemble_codeword_neon(uint8x8_t signs, int16x8_t chords, uint8x8_t steps) {
    uint8x8_t signs_shifted = vshl_n_u8(signs, 7);
    uint8x8_t chords_shifted = vshl_n_u8(vreinterpret_u8_s8(vmovn_s16(chords)), 4);
    return vorr_u8(signs_shifted, vorr_u8(chords_shifted, steps));
}

uint8x8_t get_small_codewords(int16x8_t magnitudes, uint8x8_t signs) {
    uint8x8_t magnitudes_u8 = vreinterpret_u8_s8(vmovn_s16(magnitudes));
    uint8x8_t small_codewords = vorr_u8(vshr_n_u8(magnitudes_u8, 1), vshl_n_u8(signs, 7));
    return small_codewords;
}

/**
 * Compress 16-bit signed integer samples to 8-bit A-law codewords using SIMD.
 * @param samples input array of 16-bit signed integers
 * @param codewords output array of 8-bit A-law encoded values
 * @param num_samples number of samples to encode
 */
void a_law_encode_neon(int16_t *samples, uint8_t *codewords, int num_samples) {
    int num_16x8 = (num_samples + 7) / 8;
    int idx = 0;
    for (int i = 0; i < num_16x8; i++) {
        idx = i * 8;
        // Load 8 signed 16-bit samples into a NEON vector
        int16x8_t vec_samples = vld1q_s16(&samples[idx]);

        uint8x8_t vec_signs = get_sign_neon(vec_samples);
        int16x8_t vec_magnitudes = get_magnitude_neon(vec_samples);
        int16x8_t vec_chords = get_chord_neon(vec_magnitudes);
        uint8x8_t vec_steps = get_step_neon(vec_magnitudes, vec_chords);

        // Assemble the A-law codeword
        uint8x8_t vec_codewords = assemble_codeword_neon(vec_signs, vec_chords, vec_steps);
        uint8x8_t inversion_mask = vdup_n_u8(INVERSION_MASK);
        vec_codewords = veor_u8(vec_codewords, inversion_mask);

        // Handle small magnitudes
        uint8x8_t small_flags = vmovn_u16(vcltq_s16(vec_magnitudes, vdupq_n_s16(0b10000)));
		uint8x8_t vec_codewords_small = get_small_codewords(vec_magnitudes, vec_signs);
		vec_codewords_small = veor_u8(vec_codewords_small, inversion_mask);

        // Combine the small codewords with the main codewords
        uint8x8_t final_codewords = vbsl_u8(
            small_flags,
            vec_codewords_small,
            vec_codewords
        );

        // Store the result
        vst1_u8(&codewords[idx], final_codewords);
    }

}
