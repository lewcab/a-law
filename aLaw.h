#ifndef ALAW_FUNCS_H
#define ALAW_FUNCS_H

// Define structures
typedef struct {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
} RIFFHeader;

typedef struct {
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} FMTSubchunk;

typedef struct {
    char subchunk2ID[4];
    uint32_t subchunk2Size;
    int16_t* data;
} DataSubchunk;

typedef struct {
    RIFFHeader riffHeader;
    FMTSubchunk fmtSubchunk;
    DataSubchunk dataSubchunk;
} WAVFile;

#define INVERSION_MASK 0x55 // Mask for inverting A-law codewords

// Function declarations
const char* get_bin_str(uint16_t value, int num_bits);
uint8_t a_law_encode(int16_t sample);
int16_t a_law_decode(uint8_t codeword);

#endif //ALAW_FUNCS_H
