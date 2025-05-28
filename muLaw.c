#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct {
    // Master RIFF chunk
    char fileTypeBockID[4];        // "RIFF"
    uint32_t chunkSize;     // File size - 8 bytes
    char format[4];         // "WAVE"
    // Format chunk
    char formatBlockID[4];    // "fmt"
    uint32_t subchunk1Size; // Size of the fmt chunk
    uint16_t audioFormat;   // Audio format (1=PCM; 3=IEEE 754 float)
    uint16_t numChannels;   // Number of channels
    uint32_t sampleRate;    // Sampling rate (Hz)
    uint32_t bytePerSec;    // # bytes to read per second
    uint16_t bytePerBlock;  // # bytes per block
    uint16_t bitsPerSample; // # bits per sample
    // Data chunk
    char dataBlockID[4];    // "data"
    uint32_t dataSize; // Size of the data chunk
    // Data ...
} WAVHeader;

typedef struct
{
    WAVHeader header;
    uint8_t* data;          // Pointer to the audio data
} WAVFile;


WAVFile* parseWAVFile(const char* path);
WAVHeader* parseWAVHeader(const char* path);
void printWAVFile(WAVFile* content);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    const char* inputPath = argv[1];

    WAVFile* wavFile = parseWAVFile(inputPath);

    printWAVFile(wavFile);

    free(wavFile);
    return 0;
}


WAVFile* parseWAVFile(const char* path) {
    WAVFile* content = malloc(sizeof(WAVFile));
    if (content == NULL) {
        perror("Error allocating memory for WAV file content");
        return NULL;
    }

    WAVHeader* header = parseWAVHeader(path);
    if (header == NULL) {
        free(content);
        return NULL;
    }

    content->data = malloc(header->dataSize);
    if (content->data == NULL) {
        perror("Error allocating memory for audio data");
        free(header);
        free(content);
        return NULL;
    }

    content->header = *header;
    free(header);

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        free(content->data);
        free(content);
        return NULL;
    }

    // Skip the header part in the file
    fseek(file, sizeof(WAVHeader), SEEK_SET);

    // Read the audio data
    if (fread(content->data, content->header.dataSize, 1, file) != 1) {
        perror("Error reading audio data");
        fclose(file);
        free(content->data); // Free allocated memory for audio data
        free(content);       // Free allocated memory for content
        return NULL;
    }

    fclose(file);
    return content;
}


WAVHeader* parseWAVHeader(const char* path) {
    WAVHeader* header = malloc(sizeof(WAVHeader));
    if (header == NULL) {
        perror("Error allocating memory for header data");
        return NULL;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        free(header); // Free allocated memory
        return NULL;
    }

    if (fread(header, sizeof(WAVHeader), 1, file) != 1) {
        perror("Error reading WAV header");
        fclose(file);
        free(header); // Free allocated memory
        return NULL;
    }

    fclose(file);
    return header;
}


void printWAVFile(WAVFile* content)
{
    if (content == NULL) {
        fprintf(stderr, "Header data is NULL.\n");
        return;
    }

    WAVHeader *header = &content->header;

    printf("HEADER INFORMATION:\n");
    printf("\tChunk ID: %.4s\n", header->fileTypeBockID);
    printf("\tChunk Size: %u\n", header->chunkSize);
    printf("\tFormat: %.4s\n", header->format);
    printf("\tSubchunk1 ID: %.4s\n", header->formatBlockID);
    printf("\tSubchunk1 Size: %u\n", header->subchunk1Size);
    printf("\tAudio Format: %u\n", header->audioFormat);
    printf("\tNumber of Channels: %u\n", header->numChannels);
    printf("\tSample Rate: %u Hz\n", header->sampleRate);
    printf("\tByte Rate: %u bytes/sec\n", header->bytePerSec);
    printf("\tBlock Align: %u bytes\n", header->bytePerBlock);
    printf("\tBits per Sample: %u bits\n", header->bitsPerSample);
    printf("\tSubchunk2 ID: %.4s\n", header->dataBlockID);
    printf("\tSubchunk2 Size: %u bytes\n", header->dataSize);

    printf("AUDIO DATA (%u bytes):\n", header->dataSize);
    if (content->data != NULL) {
        // Print the first few bytes of audio data as an example
        for (uint32_t i = 0; i < 10 && i < content->header.dataSize; i++) {
            printf("%02X ", content->data[i]);
        }
        printf("\n");
    } else {
        printf("No audio data loaded.\n");
    }
}