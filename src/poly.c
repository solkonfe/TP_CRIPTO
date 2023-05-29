#include "../include/poly.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

Poly * initPoly(int n){
    if (n <= 0)
        return NULL; 
    
    Poly* poly = (Poly*)malloc(sizeof(Poly));
    poly->degree = n - 1;
    poly->coefficients = (uint8_t*)malloc(n * sizeof(uint8_t));

    return poly;
}

Poly * polyFromCoef(int n, ...) {

    Poly * poly = initPoly(n);
 
    va_list args;
    va_start(args, n);
    
    for (int i = 0; i < n; i++) {
        int arg = va_arg(args, int); 
        uint8_t coef = (uint8_t)(CONG(arg));
        poly->coef[i] = coef;
    }
    va_end(args);
    return poly;
}

Poly * polyFromBytes(int n, uint8_t * bytes) {

    Poly * poly = initPoly(n);

    for (int i=0; i<n; i++) 
        poly->coef[i] = CONG(bytes[i]); 
    
    return poly; 
}

uint8_t evPoly(Poly * poly, int x) {
    uint32_t result = 0;
    uint32_t power = 1;
    
    uint8_t x2 = CONG(x); 
    for (int i = 0; i <= poly->degree; i++) {
        result += (uint32_t)poly->coef[i] * power;
        power = (power * x2) % MOD;
        result = result % MOD; 
    }
    
    return (uint8_t)(result);
}

void freePoly(Poly * poly) {
    free(poly->coef);
    free(poly);
}

Poly * interPoly(int n, int* xs, int* ys) {
   
    Poly * poly = initPoly(n)

    int yAux[n];
    
    for (int cant = 0; cant < n; cant++) {
        int curr = 0;
        for (int i = 0; i<n-cant; i++) {
            int y = !cant ? ys[i] : (yAux[i] - poly->coef[cant-1]) * INV(xs[i]); 
            y = CONG(y);
            yAux[i] = y; 

            int li = 1; 
            for (int j=0; j<n-cant; j++) 
                li *= i == j ? 1 : CONG(-1*xs[j]*INV(xs[i]-xs[j]));
            
            curr += CONG(y*li);
        }

        poly->coef[cant] = (uint8_t)CONG(curr); 
    }

    return poly; 
}
