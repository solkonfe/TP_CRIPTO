#include "../include/poly.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
   
    Polynomial* poly = initPoly(n);

    int count = 0; // Number of coefficients already analysed
    int yPrimes[n];  // y' cache - the y' are constructed recursively as interpolation progresses
    while (count < n) {
        int currentCoefficient = 0;

        // Reduced Lagrange algorithm. In each iteration we ignore one extra point
        int top = n-count;
        for (int i = 0; i<top; i++) {
            int y = !count ? y_values[i] : (yPrimes[i] - poly->coef[count-1]) * INV(x_values[i]); 
            y = TO_POSITIVE(y);
            yPrimes[i] = y; 
            int li = 1; 
            for (int j=0; j<top; j++) {
                li *= i == j ? 1 : TO_POSITIVE(-1*x_values[j]*INV(x_values[i]-x_values[j]));
                li = TO_POSITIVE(li); 
            }
            currentCoefficient += TO_POSITIVE(y*li);
        }
        poly->coef[count++] = (uint8_t)TO_POSITIVE(currentCoefficient); 
    }

    return poly; 
}

void printPoly(Polynomial* poly) {
    printf("p(x) = ");
    
    printf("%d", poly->coef[0]);
    for (int i = 1; i < poly->degree+1; i++) 
        printf(" + %dx^%d", poly->coef[i], i);
    
    printf("\n");
}

bool validCoefficients (Poly * a, Poly * b) {
    for (int i=0; i < MOD; i++ ){
        if (CONG(a[0] * i + b[0]) == 0 && CONG(a[1] * i + b[1]) == 0){
            return true;
        }
    }
    freePoly(a);
    freePoly(b);
    return false;
}

uint8_t* getSecret(uint8_t ** shadows, int * x_values, int length, int k) {
    int secretLength = shadowLength * (k - 1);
    int blockSize = BLOCK_SIZE(k);
    uint8_t * secret = (uint8_t*)malloc(sizeof(uint8_t) * secretLength);
    int current = 0;

    for (int i=0; i < length; i+=2, current++){
        //par
        int ys1[k];
        //impar
        int ys2[k];

        for (int j=0; j< k ; j++){
            ys1[j] = shadows[j][current * 2];
            ys2[j] = shadows[j][current * 2 + 1];
        }

        Poly * a = interPoly(k. x_values, ys1);
        Poly * b = interPoly(k. x_values, ys2);

        if (!validCoefficients(a, b)){
            return NULL;
        }

        for (int t=0; t< k : t++){
            secret[current * blockSize + t] = a->coeff[t];
            if( t >= 2){
                secret[current * blockSize + k + t -2] = b->coeff[t];
            }
        }

        freePoly(fi);
        freePoly(gi);
    }
    return secret;
}

uint8_t ** initShadows (int n, int size){
    uint8_t** shadows = (uint8_t ** )malloc(sizeof (uint8_t *) * n);
    for (int i=0 ; i < n ; i++){
        shadows [i] = (uint8_t*)malloc(sizeof(uint8_t) * shadowSize);
    }
    return shadows;
}

void generateShadows(uint8_t * secret, int length, int k, int n){
    int blockSize = BLOCK_SIZE(k);
    int size = length / (k-1);
    uint8_t shadows = initShadows(n, size);

    //divide secret
    int shadowPoints = 0;
    for (int i=0; i < length; i+=size, shadowPoints+=2){
        int auxa0 = secret[i];
        int auxa1 = secret[i + 1];

        secret[i] = TO_POSITIVE(secret[i]) ? secret[i] : 1;
        secret[i + 1] = TO_POSITIVE(secret[i + 1]) ? secret[i + 1] : 1;

        Poly * a = polyFromBytes(k, &secret[i]);
        Poly * b = polyFromBytes(k, &secret[i + k -2]);

        uint8_t random = (rand() % (MOD - 2)) + 1;

        b->coef[0] = TO_POSITIVE(- (TO_POSITIVE(random) * secret[i]));
        b->coef[1] = TO_POSITIVE(- (TO_POSITIVE(random) * secret[i+1]));

        secret[i] = auxa0;
        secret[i+1] = auxa1;


        //save shadows 
        for (int j = 0; j < n; j++){
            shadows[j][shadowPoints] = evaluatePolynomial(a,j+1);
            shadows[j][shadowPoints + 1] = evaluatePolynomial(b,j+1);
        }

        freePoly(a);
        freePoly(b);
    }
    return shadows;
}


