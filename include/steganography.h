#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H
#include <stdint.h>

uint8_t * hide(uint8_t * secret, int secretLength, uint8_t * originalBytes, int bitsPerByte);

uint8_t * read(uint8_t * hidden, int secretLength, int bitsPerByte, uint8_t * output);

#endif // STEGANOGRAPHY_H