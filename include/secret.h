#ifndef SECRET_H
#define SECRET_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "poly.h"
#include "utils.h"
#include "bmp.h"

uint8_t ** initShadows (int n, int size);
uint8_t* getSecret(uint8_t ** shadows, int * xs, int length, int k);
void generateShadows(uint8_t * secret, int length, int k, int n);

#endif // SECRET_H