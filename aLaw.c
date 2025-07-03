#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 512

typedef struct {
    char     chunkID[4];     // "RIFF"
    uint32_t chunkSize;
    char     format[4];      // "WAVE"
} RIFFHeader;

typedef struct {
    char     subchunk1ID[4]; // "fmt "
    uint32_t subchunk1Size;  // 16 for PCM
    uint16_t audioFormat;    // PCM = 1
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
} FMTSubchunk;

typedef struct {
    char     subchunk2ID[4]; // "data"
    uint32_t subchunk2Size;
    int16_t* data;           // Pointer to actual audio data
} DataSubchunk;

typedef struct {
    RIFFHeader riffHeader;
    FMTSubchunk fmtSubchunk;
    DataSubchunk dataSubchunk;
} WAVFile;

int parseWAV(const char *filename, WAVFile *wav) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Cannot open file");
        return 1;
    }

    // Read RIFF header
    if (fread(&wav->riffHeader, sizeof(RIFFHeader), 1, fp) != 1) {
        fprintf(stderr, "Failed to read RIFF header\n");
        fclose(fp);
        return 1;
    }

    // Validate RIFF header
    if (strncmp(wav->riffHeader.chunkID, "RIFF", 4) != 0 || strncmp(wav->riffHeader.format, "WAVE", 4) != 0) {
        fprintf(stderr, "Not a valid WAV file\n");
        fclose(fp);
        return 1;
    }

    // Read chunks until we find "fmt "
    while (1) {
        char chunkId[4];
        uint32_t chunkSize;
        if (fread(chunkId, 1, 4, fp) != 4) break;
        if (fread(&chunkSize, 4, 1, fp) != 1) break;

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            memcpy(wav->fmtSubchunk.subchunk1ID, chunkId, 4);
            wav->fmtSubchunk.subchunk1Size = chunkSize;
            if (fread(&wav->fmtSubchunk.audioFormat, chunkSize, 1, fp) != 1) {
                fprintf(stderr, "Failed to read fmt chunk\n");
                fclose(fp);
                return 1;
            }
            // Validate the format: Mono and 16 bits per sample
            if (wav->fmtSubchunk.numChannels != 1 || wav->fmtSubchunk.bitsPerSample != 16) {
                fprintf(stderr, "Unsupported WAV format. Only mono (1 channel) and 16 bits per sample are supported.\n");
                fclose(fp);
                return 1;
            }
            break;
        } else {
            // Skip this chunk
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    // Read chunks until we find "data"
    while (1) {
        char chunkId[4];
        uint32_t chunkSize;
        if (fread(chunkId, 1, 4, fp) != 4) break;
        if (fread(&chunkSize, 4, 1, fp) != 1) break;

        if (strncmp(chunkId, "data", 4) == 0) {
            memcpy(wav->dataSubchunk.subchunk2ID, chunkId, 4);
            wav->dataSubchunk.subchunk2Size = chunkSize;

            // Allocate memory for the audio data
            wav->dataSubchunk.data = (uint16_t*)malloc(chunkSize);
            if (!wav->dataSubchunk.data) {
                fprintf(stderr, "Failed to allocate memory for audio data\n");
                fclose(fp);
                return 1;
            }

            // Read the audio data
            if (fread(wav->dataSubchunk.data, 1, chunkSize, fp) != chunkSize) {
                fprintf(stderr, "Failed to read audio data\n");
                free(wav->dataSubchunk.data);
                fclose(fp);
                return 1;
            }
            break;
        } else {
            // Skip this chunk
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    fclose(fp);
    return 0;
}

void printWAVInfo(const WAVFile *wav) {
    printf("Audio Format: %u\n", wav->fmtSubchunk.audioFormat);
    printf("Channels: %u\n", wav->fmtSubchunk.numChannels);
    printf("Sample Rate: %u\n", wav->fmtSubchunk.sampleRate);
    printf("Byte Rate: %u\n", wav->fmtSubchunk.byteRate);
    printf("Block Align: %u\n", wav->fmtSubchunk.blockAlign);
    printf("Bits Per Sample: %u\n", wav->fmtSubchunk.bitsPerSample);
    printf("Data Size: %u bytes\n", wav->dataSubchunk.subchunk2Size);
    printf("Duration: %.2f seconds\n", (float)wav->dataSubchunk.subchunk2Size / wav->fmtSubchunk.byteRate);
}

// Print a 16-bit value in signed 2's complement binary format
void print_binary(uint16_t value, int num_bits) {
    for (int i = num_bits - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input.wav>\n", argv[0]);
        return 1;
    }

    WAVFile wav;
    if (parseWAV(argv[1], &wav) != 0) {
        return 1;
    }

    printWAVInfo(&wav);

    int total_samples = wav.dataSubchunk.subchunk2Size / sizeof(uint16_t);
    int num_chunks = (total_samples + CHUNK_SIZE - 1) / CHUNK_SIZE;


    // Break down the audio into 512 chunks and find their amplitudes
    for (int chunk_idx = 0; chunk_idx < num_chunks; chunk_idx++) {
        int start = chunk_idx * CHUNK_SIZE;
        int end = start + CHUNK_SIZE;
        if (end > total_samples) end = total_samples;

        printf("Chunk %d (%d samples):\n", chunk_idx, end - start);
        for (int i = start; i < end; i++) {
            int16_t sample = wav.dataSubchunk.data[i];
            // i go printf("Sample %3d: %6d (bin: ", i, sample);
            // print_binary(sample, 16);
            // printf(")\n");
            uint8_t compressed = a_law_compression(sample);
            printf("Sample %6d -> A-law: %3d (bin: ", sample, compressed);
            print_binary(compressed, 8);
            printf(")\n");
        }
    }

    // Cleanup
    free(wav.dataSubchunk.data);

    return 0;
}