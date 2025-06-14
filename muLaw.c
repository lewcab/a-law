#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

void print_binary(uint16_t value) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
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

    for (int i = 0; i < wav.dataSubchunk.subchunk2Size / sizeof(uint16_t); i++) {
        if (i < 100) {
            printf("Sample %3d:  (%3d)\n", i, wav.dataSubchunk.data[i]);
        }
    }

    // Cleanup
    free(wav.dataSubchunk.data);

    return 0;
}