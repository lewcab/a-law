#ifndef ALAW_H
#define ALAW_H

// Define structures
typedef struct {
    char chunk_id[4];
    uint32_t file_size;
    char format_id[4];
} RIFFChunk;

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t bytes_per_block;
    uint16_t bits_per_sample;
} FormatChunk;

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    int16_t* data;
} DataChunk;

typedef struct {
    RIFFChunk riff_chunk;
    FormatChunk format_chunk;
    DataChunk data_chunk;
} WAVFile;

#define INVERSION_MASK 0x55 // Mask for inverting A-law codewords
#define MAGNITUDE_SHIFT 4 // Shift to fit 12-bit magnitude into 16 bits and vice versa
#define PCM_FORMAT 0x01 // PCM format identifier
#define ALAW_FORMAT 0x06 // A-Law format identifier
#define PCM_SAMPLE_SIZE 16 // Sample size in bits for PCM
#define ALAW_SAMPLE_SIZE 8 // Sample size in bits for A-law

// Function declarations
uint16x8_t get_sign_neon(int16x8_t samples);
int16x8_t get_magnitude_neon(int16x8_t samples);
int16x8_t get_chord_neon(uint16x8_t magnitudes);
int16x8_t get_step_neon(int16x8_t magnitudes, int16x8_t chords);
uint8x8_t assemble_codeword_neon(int16x8_t signs, int16x8_t chords, int16x8_t steps);
void a_law_encode_neon(int16_t *samples, int8_t *codewords, int num_samples);

#endif //ALAW_H
