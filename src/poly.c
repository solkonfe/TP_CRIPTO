#include "../include/poly.h"

Poly * initPoly(int n){
    if (n <= 0)
        return NULL; 
    
    Poly* poly = (Poly*)malloc(sizeof(Poly));
    poly->degree = n - 1;
    poly->coef = (uint8_t*)malloc(n * sizeof(uint8_t));

    return poly;
}

Poly * polyFromCoef(int n, ...){
    if (n <= 0)
        return NULL; 
    
    va_list args;
    va_start(args, n);
    
    Poly* poly = initPoly(n);

    for (int i = 0; i < n; i++) {
        int arg = va_arg(args, int); 
        uint8_t coeff = (uint8_t)(TO_POSITIVE(arg));
        poly->coef[i] = coeff;
    }
    
    va_end(args);

    return poly;
}

Poly * polyFromBytes(int n, uint8_t * bytes) {

    Poly * poly = initPoly(n);

    for (int i=0; i<n; i++) 
        poly->coef[i] = TO_POSITIVE(bytes[i]); 
    
    return poly; 
}

uint8_t evaluatePolynomial(Poly * poly, int x) {
    uint32_t result = 0;
    uint32_t power = 1;
    
    uint8_t x2 = TO_POSITIVE(x); 

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

Poly * interPoly(int n, int* x_values, int* y_values) {
   
    Poly * poly = initPoly(n);

    int count = 0;
    int yAux[n];
    while (count < n) {
        int current = 0;
        int max = n-count;
        for (int i = 0; i<max; i++) {
            int y = !count ? y_values[i] : (yAux[i] - poly->coef[count-1]) * INV(x_values[i]); 
            y = TO_POSITIVE(y);
            yAux[i] = y; 
            int li = 1; 
            for (int j=0; j<max; j++) {
                li *= i == j ? 1 : TO_POSITIVE(-1*x_values[j]*INV(x_values[i]-x_values[j]));
                li = TO_POSITIVE(li); 
            }
            current += TO_POSITIVE(y*li);
        }
        poly->coef[count++] = (uint8_t)TO_POSITIVE(current); 
    }
    
    return poly; 
}

void printPoly(Poly* poly) {
    printf("p(x) = ");
    
    printf("%d", poly->coef[0]);
    for (int i = 1; i < poly->degree+1; i++) 
        printf(" + %dx^%d", poly->coef[i], i);
    
    printf("\n");
}