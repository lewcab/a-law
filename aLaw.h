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

// Function declarations
int parseWAV(const char *filename, WAVFile *wav);
void printWAVInfo(const WAVFile *wav);
uint8_t a_law_encode(int16_t sample);
const char* get_bin_str(uint16_t value, int num_bits);

#endif //ALAW_FUNCS_H
