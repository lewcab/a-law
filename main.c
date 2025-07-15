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
    if (fread(&wav->riff_chunk, sizeof(RIFFChunk), 1, fp) != 1) {
        fprintf(stderr, "Failed to read RIFF header\n");
        fclose(fp);
        return 1;
    }

    // Validate RIFF header
    if (strncmp(wav->riff_chunk.chunk_id, "RIFF", 4) != 0 || strncmp(wav->riff_chunk.format_id, "WAVE", 4) != 0) {
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
            memcpy(wav->format_chunk.chunk_id, chunkId, 4);
            wav->format_chunk.chunk_size = chunkSize;
            if (fread(&wav->format_chunk.audio_format, chunkSize, 1, fp) != 1) {
                fprintf(stderr, "Failed to read fmt chunk\n");
                fclose(fp);
                return 1;
            }
            // Validate the format: Mono and 16 bits per sample
            if (wav->format_chunk.num_channels != 1 || wav->format_chunk.bits_per_sample != 16) {
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
            memcpy(wav->data_chunk.chunk_id, chunkId, 4);
            wav->data_chunk.chunk_size = chunkSize;

            // Allocate memory for the audio data
            wav->data_chunk.data = (uint16_t*)malloc(chunkSize);
            if (!wav->data_chunk.data) {
                fprintf(stderr, "Failed to allocate memory for audio data\n");
                fclose(fp);
                return 1;
            }

            // Read the audio data
            if (fread(wav->data_chunk.data, 1, chunkSize, fp) != chunkSize) {
                fprintf(stderr, "Failed to read audio data\n");
                free(wav->data_chunk.data);
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
    printf("Audio Format: %u\n", wav->format_chunk.audio_format);
    printf("Channels: %u\n", wav->format_chunk.num_channels);
    printf("Sample Rate: %u\n", wav->format_chunk.sample_rate);
    printf("Byte Rate: %u\n", wav->format_chunk.byte_rate);
    printf("Block Align: %u\n", wav->format_chunk.bytes_per_block);
    printf("Bits Per Sample: %u\n", wav->format_chunk.bits_per_sample);
    printf("Data Size: %u bytes\n", wav->data_chunk.chunk_size);
    printf("Duration: %.2f seconds\n", (float)wav->data_chunk.chunk_size / wav->format_chunk.byte_rate);
}


int write_wav(const WAVFile *wav, const char *output_filename) {
    FILE *fp = fopen(output_filename, "wb");
    if (!fp) {
        perror("Cannot open output file");
        return 1;
    }

    // Write RIFF header
    if (fwrite(&wav->riff_chunk, sizeof(RIFFChunk), 1, fp) != 1) {
        fprintf(stderr, "Failed to write RIFF header\n");
        fclose(fp);
        return 1;
    }

    // Write fmt subchunk
    if (fwrite(&wav->format_chunk, sizeof(FormatChunk), 1, fp) != 1) {
        fprintf(stderr, "Failed to write fmt subchunk\n");
        fclose(fp);
        return 1;
    }

    // Write data subchunk header
    if (fwrite(&wav->data_chunk.chunk_id, sizeof(wav->data_chunk.chunk_id), 1, fp) != 1 ||
        fwrite(&wav->data_chunk.chunk_size, sizeof(wav->data_chunk.chunk_size), 1, fp) != 1) {
        fprintf(stderr, "Failed to write data subchunk header\n");
        fclose(fp);
        return 1;
        }

    // Write audio data
    if (fwrite(wav->data_chunk.data, 1, wav->data_chunk.chunk_size, fp) != wav->data_chunk.chunk_size) {
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

    int total_samples = wav.data_chunk.chunk_size / sizeof(uint16_t);
    int num_chunks = (total_samples + CHUNK_SIZE - 1) / CHUNK_SIZE;

    FILE *fp = fopen(output_filename, "ab");
    if (!fp) {
        perror("Cannot open output file for appending");
        free(wav.data_chunk.data);
        return 1;
    }
    for (int chunk_idx = 0; chunk_idx < num_chunks; chunk_idx++) {
        // Break down the audio into 512 chunks and find their amplitudes
        int start = chunk_idx * CHUNK_SIZE;
        int end = start + CHUNK_SIZE;
        if (end > total_samples) end = total_samples;
        int16_t out_buffer[CHUNK_SIZE];

        for (int i = start; i < end; i++) {
            int16_t sample = wav.data_chunk.data[i];
            uint8_t compressed = a_law_encode(sample);
            int16_t decompressed = a_law_decode(compressed);
            out_buffer[i - start] = decompressed;
        }

        if (fwrite(out_buffer, sizeof(int16_t), end - start, fp) != (end - start)) {
            fprintf(stderr, "Failed to write processed chunk to output file\n");
        }
    }

    // Cleanup
    fclose(fp);
    free(wav.data_chunk.data);

    return 0;
}
