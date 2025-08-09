#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "aLaw.h"

#define ENCODED_LUT_SIZE 4096
#define DECODED_LUT_SIZE 256
#define PER_LINE 16

static void write_encoded_c(const uint8_t *encoded) {
    FILE *fp = fopen("encoded_lut.c", "w");
    if (!fp) return;
    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "__attribute__((section(\".lut_bram\")))\n");
    fprintf(fp, "const uint8_t encoded_lut[%d] = {\n", ENCODED_LUT_SIZE);
    for (int i = 0; i < ENCODED_LUT_SIZE; i++) {
        fprintf(fp, "0x%02X", encoded[i]);
        if (i != ENCODED_LUT_SIZE - 1) fprintf(fp, ",");
        if ((i + 1) % PER_LINE == 0) fprintf(fp, "\n"); else fprintf(fp, " ");
    }
    fprintf(fp, "};\n");
    fclose(fp);
}

static void write_decoded_c(const int16_t *decoded) {
    FILE *fp = fopen("decoded_lut.c", "w");
    if (!fp) return;
    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "__attribute__((section(\".lut_bram\")))\n");
    fprintf(fp, "const uint16_t decoded_lut[%d] = {\n", DECODED_LUT_SIZE);
    for (int i = 0; i < DECODED_LUT_SIZE; i++) {
        uint16_t v = (uint16_t)decoded[i];          // remove sign extension
        fprintf(fp, "0x%04X", v);
        if (i != DECODED_LUT_SIZE - 1) fprintf(fp, ",");
        if ((i + 1) % PER_LINE == 0) fprintf(fp, "\n"); else fprintf(fp, " ");
    }
    fprintf(fp, "};\n");
    fclose(fp);
}

int main(void) {
    uint8_t encoded_lut[ENCODED_LUT_SIZE];
    int16_t decoded_lut[DECODED_LUT_SIZE];

    for (int i = 0; i < ENCODED_LUT_SIZE; i++) {
        int16_t sample = (int16_t)(i << 4);
        encoded_lut[i] = a_law_encode(sample);
    }
    for (uint16_t i = 0; i < DECODED_LUT_SIZE; i++) {
        decoded_lut[i] = a_law_decode((uint8_t)i);
    }

    write_encoded_c(encoded_lut);
    write_decoded_c(decoded_lut);

    return 0;
}