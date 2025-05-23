#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)
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
#pragma pack(pop)


int parse_header(char* path);


int main(void) {
    const char* sound = "claps";
    const char* channel = "stereo";
    char f_16[64];
    char f_32[64];

    sprintf(f_16, "inputs/%s_16_stereo.wav", sound, channel);
    sprintf(f_32, "inputs/%s_32_mono.wav", sound, channel);

    int a = parse_header(f_16);
    printf("\n");
    printf("\n");
    int b = parse_header(f_32);

}

int parse_header(char* path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    WAVHeader header;
    if (fread(&header, sizeof(WAVHeader), 1, file) != 1) {
        perror("Error reading WAV header");
        fclose(file);
        return 1;
    }

    printf("Chunk ID: %.4s\n", header.chunkID);
    printf("Chunk Size: %u\n", header.chunkSize);
    printf("Format: %.4s\n", header.format);
    printf("Subchunk1 ID: %.4s\n", header.subchunk1ID);
    printf("Subchunk1 Size: %u\n", header.subchunk1Size);
    printf("Audio Format: %u\n", header.audioFormat);
    printf("Number of Channels: %u\n", header.numChannels);
    printf("Sample Rate: %u\n", header.sampleRate);
    printf("Bits Per Second: %u\n", header.bytePerSec);
    printf("Bytes Per Block: %u\n", header.bytePerBlock);
    printf("Bits Per Sample: %u\n", header.bitsPerSample);
    printf("Subchunk2 ID: %.4s\n", header.subchunk2ID);
    printf("Subchunk2 Size: %u\n", header.subchunk2Size);

    fclose(file);
    return 0;
}