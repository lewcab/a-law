#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aLaw.h"

// Define constants
#define CHUNK_SIZE 512


int parse_wav(const char *filename, WAVFile *wav) {
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


void print_wav_info(const WAVFile *wav) {
    printf("Audio Format: %u\n", wav->fmtSubchunk.audioFormat);
    printf("Channels: %u\n", wav->fmtSubchunk.numChannels);
    printf("Sample Rate: %u\n", wav->fmtSubchunk.sampleRate);
    printf("Byte Rate: %u\n", wav->fmtSubchunk.byteRate);
    printf("Block Align: %u\n", wav->fmtSubchunk.blockAlign);
    printf("Bits Per Sample: %u\n", wav->fmtSubchunk.bitsPerSample);
    printf("Data Size: %u bytes\n", wav->dataSubchunk.subchunk2Size);
    printf("Duration: %.2f seconds\n", (float)wav->dataSubchunk.subchunk2Size / wav->fmtSubchunk.byteRate);
}


int write_wav(const WAVFile *wav, const char *output_filename) {
    FILE *fp = fopen(output_filename, "wb");
    if (!fp) {
        perror("Cannot open output file");
        return 1;
    }

    // Write RIFF header
    if (fwrite(&wav->riffHeader, sizeof(RIFFHeader), 1, fp) != 1) {
        fprintf(stderr, "Failed to write RIFF header\n");
        fclose(fp);
        return 1;
    }

    // Write fmt subchunk
    if (fwrite(&wav->fmtSubchunk, sizeof(FMTSubchunk), 1, fp) != 1) {
        fprintf(stderr, "Failed to write fmt subchunk\n");
        fclose(fp);
        return 1;
    }

    // Write data subchunk header
    if (fwrite(&wav->dataSubchunk.subchunk2ID, sizeof(wav->dataSubchunk.subchunk2ID), 1, fp) != 1 ||
        fwrite(&wav->dataSubchunk.subchunk2Size, sizeof(wav->dataSubchunk.subchunk2Size), 1, fp) != 1) {
        fprintf(stderr, "Failed to write data subchunk header\n");
        fclose(fp);
        return 1;
        }

    // Write audio data
    if (fwrite(wav->dataSubchunk.data, 1, wav->dataSubchunk.subchunk2Size, fp) != wav->dataSubchunk.subchunk2Size) {
        fprintf(stderr, "Failed to write audio data\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.wav> <output.wav>\n", argv[0]);
        return 1;
    }
    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    printf("input: %s\n", input_filename);
    printf("output: %s\n", output_filename);

    WAVFile wav;
    if (parse_wav(input_filename, &wav) != 0) {
        return 1;
    }

    print_wav_info(&wav);

    write_wav(&wav, output_filename);

    int total_samples = wav.dataSubchunk.subchunk2Size / sizeof(uint16_t);
    int num_chunks = (total_samples + CHUNK_SIZE - 1) / CHUNK_SIZE;

    // Break down the audio into 512 chunks and find their amplitudes
    for (int chunk_idx = 0; chunk_idx < num_chunks; chunk_idx++) {
        int start = chunk_idx * CHUNK_SIZE;
        int end = start + CHUNK_SIZE;
        if (end > total_samples) end = total_samples;
        int16_t out_buffer[CHUNK_SIZE];

        for (int i = start; i < end; i++) {
            int16_t sample = wav.dataSubchunk.data[i];
            uint8_t compressed = a_law_encode(sample);
            int16_t decompressed = a_law_decode(compressed);
            out_buffer[i] = decompressed;
        }

        // Write the processed chunk to the output file
        FILE *fp = fopen(output_filename, "ab");
        if (fwrite(out_buffer, sizeof(int16_t), end - start, fp) != (end - start)) {
            fprintf(stderr, "Failed to write processed chunk to output file\n");
            fclose(fp);
        }
    }

    // Cleanup
    free(wav.dataSubchunk.data);

    return 0;
}
