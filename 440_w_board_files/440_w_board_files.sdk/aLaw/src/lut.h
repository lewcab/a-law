#ifndef LUT_H
#define LUT_H

#include <stdint.h>

#define ENCODED_LUT_SIZE 4096
#define DECODED_LUT_SIZE 256

extern const uint8_t encoded_lut[ENCODED_LUT_SIZE];
extern const int16_t decoded_lut[DECODED_LUT_SIZE];

#endif // LUT_H
