#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct {
    // Master RIFF chunk
    char chunkID[4];        // "RIFF"
    uint32_t chunkSize;     // File size - 8 bytes
    char format[4];         // "WAVE"
    // Format chunk
    char subchunk1ID[4];    // "fmt"
    uint32_t subchunk1Size; // Size of the fmt chunk
    uint16_t audioFormat;   // Audio format (1=PCM; 3=IEEE 754 float)
    uint16_t numChannels;   // Number of channels
    uint32_t sampleRate;    // Sampling rate (Hz)
    uint32_t bytePerSec;    // # bytes to read per second
    uint16_t bytePerBlock;  // # bytes per block
    uint16_t bitsPerSample; // # bits per sample
    // Data chunk
    char subchunk2ID[4];    // "data"
    uint32_t subchunk2Size; // Size of the data chunk
    // Data ...
} WAVHeader;


WAVHeader* parse_header(const char* path);
void print_header(WAVHeader* header);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];

    WAVHeader* header_data = parse_header(input_path);

    print_header(header_data);

    free(header_data);
    return 0;
}


WAVHeader* parse_header(const char* path) {
    WAVHeader* header_data = malloc(sizeof(WAVHeader));
    if (header_data == NULL) {
        perror("Error allocating memory for header data");
        return NULL;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        free(header_data); // Free allocated memory
        return NULL;
    }

    if (fread(header_data, sizeof(WAVHeader), 1, file) != 1) {
        perror("Error reading WAV header");
        fclose(file);
        free(header_data); // Free allocated memory
        return NULL;
    }

    fclose(file);
    return header_data;
}


void print_header(WAVHeader* header) {
    if (header == NULL) {
        fprintf(stderr, "Header data is NULL.\n");
        return;
    }

    printf("Chunk ID: %.4s\n", header->chunkID);
    printf("Chunk Size: %u\n", header->chunkSize);
    printf("Format: %.4s\n", header->format);
    printf("Subchunk1 ID: %.4s\n", header->subchunk1ID);
    printf("Subchunk1 Size: %u\n", header->subchunk1Size);
    printf("Audio Format: %u\n", header->audioFormat);
    printf("Number of Channels: %u\n", header->numChannels);
    printf("Sample Rate: %u Hz\n", header->sampleRate);
    printf("Byte Rate: %u bytes/sec\n", header->bytePerSec);
    printf("Block Align: %u bytes\n", header->bytePerBlock);
    printf("Bits per Sample: %u bits\n", header->bitsPerSample);
    printf("Subchunk2 ID: %.4s\n", header->subchunk2ID);
    printf("Subchunk2 Size: %u bytes\n", header->subchunk2Size);
}