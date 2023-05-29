#include "../include/steganography.h"
#include <limits.h>

#define LSB(x,n) ( ((x) >> (n) << (n)) ^ (x) ) 
#define MSB(x,n) ( (x) >> (CHAR_BIT - (n) ) )
#define SET_LSBS_TO_ZERO(x, n) ( (x) >> (n) << (n) ) 

//https://codereview.stackexchange.com/questions/258986/png-steganography-tool-in-c

uint8_t * hide_byte(uint8_t * original, uint8_t secret_byte, uint8_t bitsPerByte)
{
    uint8_t * limit = original + (CHAR_BIT / bitsPerByte);
    
    for ( ; original < limit; original++){
        uint8_t aux = * original;
        aux = SET_LSBS_TO_ZERO(aux, bitsPerByte);

        uint8_t toHide = MSB (secret_byte, bitsPerByte);
        aux = aux | toHide;
        *original = aux;

        toHide <<= bitsPerByte;
    }
    return original;
}

uint8_t * hide(uint8_t * secret, int secretLength, uint8_t * originalBytes, int bitsPerByte)
{
    for (int i = 0 ; i < secretLength ; i++) {
        originalBytes = hide_byte(originalBytes, secret[i], bitsPerByte);
    }
    return originalBytes;
}

uint8_t read_byte(uint8_t ** bytes, uint8_t bitsPerByte)
{
    uint8_t byte;
    uint8_t * aux = *bytes;
    uint8_t * limit = bytes + (CHAR_BIT / bitsPerByte);

    for (byte = 0; aux < limit; aux++) {
        uint8_t auxBytes = *aux;
        uint8_t hidden = LSB(auxBytes, bitsPerByte);
        byte <<= bitsPerByte;
        byte |= hidden;
    }

    *bytes = aux;
    return byte;
}

uint8_t * read(uint8_t * hidden, int secretLength, int bitsPerByte, uint8_t * output) {
    for(int bytes = 0; bytes < secretLength; bytes++) {
        output[bytes] = read_byte(&hidden, bitsPerByte);
    }
    return output;
}


