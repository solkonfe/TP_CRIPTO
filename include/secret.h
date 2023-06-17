#ifndef SECRET_H
#define SECRET_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "poly.h"
#include "utils.h"
#include "bmp.h"

uint8_t ** initShadows (int n, int size);
void getSecret(shadow ** shadows, int k, bmpFile* file, struct params * params);
void generateShadows(uint8_t * secret, int length, int k, int n);
void recover(struct params * params) ;
void distribute(struct params * params);

#endif // SECRET_H
