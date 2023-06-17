#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "secret.h"
#include "utils.h"

typedef struct Poly {
    uint8_t* coef; 
    uint8_t degree; 
} Poly;

Poly * interPoly(int n, uint8_t * x_values, uint8_t * y_values);
void freePoly(Poly * poly);
uint8_t evaluatePolynomial(Poly * poly, int x);
Poly * polyFromBytes(int n, uint8_t * bytes);
Poly * polyFromCoef(int n, ...);


#endif // POLY_H
