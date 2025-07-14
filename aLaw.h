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

// Function declarations
const char* get_bin_str(uint16_t value, int num_bits);
int get_sign(int16_t sample);
uint16_t get_magnitude(int16_t sample);
int get_chord(uint16_t magnitude);
int get_step(int magnitude, int chord);
uint8_t a_law_encode(int16_t sample);
int16_t a_law_decode(uint8_t codeword);

#endif //ALAW_H
