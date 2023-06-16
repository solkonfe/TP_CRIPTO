#include "../include/secret.h"

bool validCoefficients (Poly * a, Poly * b);


void recover(struct * params){
    uint8_t ** shadows = initShadows(n, size); //malloc de shadows
    bmpFile * currentImageFile;
    for (int i=0; i < params->k; i++){
        //funcion openDirectory
        currentImageFile = openBmpFile(params->directory);

    }

}

//https://c-for-dummies.com/blog/?p=3246#:~:text=The%20directory%20holds%20all%20that,It%27s%20prototyped%20in%20the%20dirent.
void makePath(struct * params){
    DIR * directory = openDir();
    if (directory == NULL){
        return;
    }
    struct dirent *entry;
    int files = 0;
    while( (entry=readdir(directory)) )
    {
        if(strcmpy(entry->d_name, ".") == 0 || strcmpy(entry->d_name, "..") == 0){
            continue;
        }
        
        files++;
        printf("File %3d: %s\n",
                files,
                entry->d_name
              );
    }

    closedir(folder);


}

uint8_t ** initShadows (int n, int size){
    uint8_t** shadows = (uint8_t ** )malloc(sizeof (uint8_t *) * n);
    //shadow[shadowNumber][pointNumber] = point
    for (int i=0 ; i < n ; i++){
        shadows[i] = (uint8_t*)malloc(sizeof(uint8_t) * size);
    }
    return shadows;
}


uint8_t * getSecret(uint8_t ** shadows, int * x_values, int length, int k) {
    int secretLength = length * (k - 1);shadad00/cripto/src/utils.c
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

        Poly * a = interPoly(k, x_values, ys1);
        Poly * b = interPoly(k, x_values, ys2);

        if (!validCoefficients(a, b)){
            return NULL;
        }

        for (int t=0; t< k ; t++){
            secret[current * blockSize + t] = a->coef[t];
            if( t >= 2){
                secret[current * blockSize + k + t -2] = b->coef[t];
            }
        }

        freePoly(a);
        freePoly(b);
    }
    return secret;
}

void generateShadows(uint8_t * secret, int length, int k, int n){
    int blockSize = BLOCK_SIZE(k);
    int size = length / (k-1);
    uint8_t ** shadows = initShadows(n, size);

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

//-------------------------------------------------
//--------HELPER FUNCTIONS-------------------------
//-------------------------------------------------

bool validCoefficients (Poly * a, Poly * b) {
    for (int i=0; i < MOD; i++ ){
        if (TO_POSITIVE(a->coef[0] * i + b->coef[0]) == 0 && TO_POSITIVE(a->coef[1] * i + b->coef[1]) == 0){
            return true;
        }
    }
    freePoly(a);
    freePoly(b);
    return false;
}