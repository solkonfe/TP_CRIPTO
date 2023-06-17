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

#define LSB4(x) (( x == 3 || x == 4 ) ? 1 : 0)
#define LSB4_BYTES(x) (( x ) ? 2 : 4)

void getSecret(shadow ** shadows, int k, bmpFile* file, struct params * params);
shadow ** generateShadows(bmpFile * file, int k, int n);
void recover(struct params * params) ;
void distribute(struct params * params);

#endif // SECRET_H
