#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aLaw.h"


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


int write_wav_alaw(const WAVFile *wav, const char *output_filename, const uint8_t *out_buffer) {
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
    if (
        fwrite(&wav->data_chunk.chunk_id, sizeof(wav->data_chunk.chunk_id), 1, fp) != 1 ||
        fwrite(&wav->data_chunk.chunk_size, sizeof(wav->data_chunk.chunk_size), 1, fp) != 1
    ) {
        fprintf(stderr, "Failed to write data subchunk header\n");
        fclose(fp);
        return 1;
    }

    // Write audio data
    if (fwrite(out_buffer, sizeof(uint8_t), wav->data_chunk.chunk_size / sizeof(uint8_t), fp) != wav->data_chunk.chunk_size / sizeof(uint8_t)) {
        fprintf(stderr, "Failed to write audio data\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}


int write_wav_pcm(const WAVFile *wav, const char *output_filename, const int16_t *out_buffer) {
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
    if (
        fwrite(&wav->data_chunk.chunk_id, sizeof(wav->data_chunk.chunk_id), 1, fp) != 1 ||
        fwrite(&wav->data_chunk.chunk_size, sizeof(wav->data_chunk.chunk_size), 1, fp) != 1
    ) {
        fprintf(stderr, "Failed to write data subchunk header\n");
        fclose(fp);
        return 1;
    }

    // Write audio data
    if (fwrite(out_buffer, sizeof(int16_t), wav->data_chunk.chunk_size / sizeof(int16_t), fp) != wav->data_chunk.chunk_size / sizeof(int16_t)) {
        fprintf(stderr, "Failed to write audio data\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}


void convert_wav_header(WAVFile *wav, int format, int bits_per_sample, int total_samples) {
    wav->format_chunk.audio_format = format;
    wav->format_chunk.bits_per_sample = bits_per_sample;
    wav->format_chunk.bytes_per_block = wav->format_chunk.num_channels * wav->format_chunk.bits_per_sample / 8;
    wav->format_chunk.byte_rate = wav->format_chunk.sample_rate * wav->format_chunk.bytes_per_block;
    wav->data_chunk.chunk_size = total_samples * wav->format_chunk.bytes_per_block;
}


uint8_t* alaw(const WAVFile *wav, int total_samples){
    uint8_t *out_buffer = malloc(total_samples * sizeof(uint8_t));
    if (!out_buffer) {
        fprintf(stderr, "Failed to allocate memory for output buffer\n");
        free(wav->data_chunk.data);
        return NULL;
    }

    for (int i = 0; i < total_samples; i++) {
        int16_t sample = wav->data_chunk.data[i];
        uint8_t encoded = a_law_encode(sample);
        out_buffer[i] = encoded;
    }

    return out_buffer;
}


int16_t* pcm(const WAVFile *wav, int total_samples) {
    int16_t *out_buffer = malloc(total_samples * sizeof(int16_t));
    if (!out_buffer) {
        fprintf(stderr, "Failed to allocate memory for output buffer\n");
        free(wav->data_chunk.data);
        return NULL;
    }

    for (int i = 0; i < total_samples; i++) {
        int16_t sample = wav->data_chunk.data[i];
        uint8_t encoded = a_law_encode(sample);
        int16_t decoded = a_law_decode(encoded);
        out_buffer[i] = decoded;
    }

    return out_buffer;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.wav> <output.wav>\n", argv[0]);
        return 1;
    }
    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    int output_format = ALAW_FORMAT;
    int bits_per_sample = ALAW_SAMPLE_SIZE;
    if (argc == 4) {
        output_format = PCM_FORMAT;
        bits_per_sample = PCM_SAMPLE_SIZE;
    }

    printf("input: %s\n", input_filename);
    printf("output: %s\n", output_filename);

    WAVFile wav;
    if (parse_wav(input_filename, &wav) != 0) {
        return 1;
    }

    printf("\nInput WAV Header\n");
    print_wav_info(&wav);

    int total_samples = wav.data_chunk.chunk_size / sizeof(int16_t);
    if (output_format == ALAW_FORMAT) {
        uint8_t *out_buffer = alaw(&wav, total_samples);
        convert_wav_header(&wav, output_format, bits_per_sample, total_samples);
        if (write_wav_alaw(&wav, output_filename, out_buffer) != 0) {
            fprintf(stderr, "Failed to write output WAV file\n");
            free(wav.data_chunk.data);
            free(out_buffer);
            return 1;
        }
        free(out_buffer);

    } else if (output_format == PCM_FORMAT) {
        int16_t *out_buffer = pcm(&wav, total_samples);
        convert_wav_header(&wav, output_format, bits_per_sample, total_samples);
        if (write_wav_pcm(&wav, output_filename, out_buffer) != 0) {
            fprintf(stderr, "Failed to write output WAV file\n");
            free(wav.data_chunk.data);
            free(out_buffer);
            return 1;
        }
        free(out_buffer);

    }

    printf("\nOutput WAV Header\n");
    print_wav_info(&wav);

    // Cleanup
    free(wav.data_chunk.data);

    return 0;
}
