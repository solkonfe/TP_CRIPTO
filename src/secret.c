#include "../include/secret.h"

bool validCoefficients(Poly* a, Poly* b);
char** makePath(struct params * params);

static shadow * fromImageToShadow(uint8_t k , bmpFile * imageFile){
    shadow * shadow = malloc(sizeof (shadow));
    shadow->shadowNumber = imageFile->header->reserved1;
    shadow->pointNumber = imageFile->header->image_size_bytes / (k -1);
    shadow->points = calloc(shadow->pointNumber,1);

    int lsb4 = ( k == 3 || k == 4 ) ? 1 : 0;
    int ImageBytesToShadowByte= ( lsb4 ) ? 2 : 4; // if lsb4 you need two uint8_t from image to generate a shadow uint8_t
    int bitOperator = lsb4 ? 0x0f:0x03; // four or two least significant bits.
    uint8_t shifter = lsb4 ? 4 : 2;


    uint64_t currentShadowBlock = 0;
    while(currentShadowBlock < shadow->pointNumber){
        for(uint64_t i = (ImageBytesToShadowByte) * currentShadowBlock ; i < (ImageBytesToShadowByte* ( currentShadowBlock + 1)); i++){
            shadow->points[currentShadowBlock] += imageFile->pixels[i]  & bitOperator;
            if (i + 1 != ((ImageBytesToShadowByte* ( currentShadowBlock + 1))) )
                shadow->points[currentShadowBlock] = shadow->points[currentShadowBlock] << shifter;
        }
        currentShadowBlock++;
    }

    return shadow;
}

void distribute(struct params *  params) {}

void recover(struct params *  params) {

    char** fileNames = makePath(params);
    shadow ** shadows = malloc(params->k * (sizeof(shadow *)));
    bmpFile* currentImageFile;

    for (int i = 0; i < params->k; i++) {
        currentImageFile = openBmpFile(fileNames[i]);
        shadows[i] = fromImageToShadow(params->k, currentImageFile);
    }

    bmpFile* file = malloc(sizeof(bmpFile*));
    int size = currentImageFile->header->size - currentImageFile->header->image_size_bytes;
    file->header = malloc(size * sizeof(uint8_t));
    memcpy(file->header, currentImageFile->header, size);
    file->pixels = calloc(file->header->image_size_bytes, 1);


    getSecret(shadows, params->k, file, params);
}

//https://c-for-dummies.com/blog/?p=3246#:~:text=The%20directory%20holds%20all%20that,It%27s%20prototyped%20in%20the%20dirent.
char** makePath(struct params * params) {
    DIR* directory = opendir(params->directory);
    if (directory == NULL) {
        exit(1);
    }

    char** fileNames = malloc(params->n * sizeof(char*));
    struct dirent* entry;
    int file = 0;
    while ((entry = readdir(directory))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        uint64_t directoryLength = strlen(params->directory);
        uint64_t fileNameLength = directoryLength + 1 + strlen(entry->d_name);
        fileNames[file] = malloc(fileNameLength + 1); // Add 1 for null terminator

        strcpy(fileNames[file], params->directory);
        strcat(fileNames[file], "/");
        strcat(fileNames[file], entry->d_name);

        file++;
    }

    return fileNames;
}

uint8_t** initShadows(int n, int size) {
    uint8_t** shadows = (uint8_t**) malloc(sizeof(uint8_t*) * n);
    for (int i = 0; i < n; i++) {
        shadows[i] = (uint8_t*) malloc(sizeof(uint8_t) * size);
    }
    return shadows;
}


void getSecret(shadow ** shadows, int k, bmpFile* file, struct params * params) {
    uint8_t * image = file->pixels;
    uint8_t current = 0;

    uint8_t* xValues = malloc(k);
    uint8_t* ys1 = malloc(k);
    uint8_t* ys2 = malloc(k);

    for (current = 0; current < (file->header->image_size_bytes); current += 2, image += (2*k) - 2) {
        for (int j = 0; j < k; j++) {
            xValues[j] = shadows[j]->shadowNumber;
            ys1[j] = shadows[j]->points[current];
            ys2[j] = shadows[j]->points[current + 1];
        }

        Poly* a = interPoly(k, xValues, ys1);
        Poly* b = interPoly(k, xValues, ys2);

        if (!validCoefficients(a, b)) {
            return;
        }

        uint8_t  * coefficient = malloc( 2* k* sizeof(uint8_t));
        memcpy(coefficient, a, k);
        memcpy(coefficient + k , b, k);

        freePoly(a);
        freePoly(b);
    }

    int size = file->header->size - file->header->image_size_bytes;
    int fd = open(params->file, O_WRONLY | O_CREAT);
    lseek(fd, 0, SEEK_SET);
    write(fd, file->header, size);
    write(fd, file->pixels, file->header->image_size_bytes);
    close(fd);
}

void generateShadows(uint8_t* secret, int length, int k, int n) {
    int blockSize = BLOCK_SIZE(k);
    int size = length / (k - 1);
    uint8_t** shadows = initShadows(n, size);

    //divide secret
    int shadowPoints = 0;
    for (int i = 0; i < length; i += size, shadowPoints += 2) {
        int auxa0 = secret[i];
        int auxa1 = secret[i + 1];

        secret[i] = TO_POSITIVE(secret[i]) ? secret[i] : 1;
        secret[i + 1] = TO_POSITIVE(secret[i + 1]) ? secret[i + 1] : 1;

        Poly* a = polyFromBytes(k, &secret[i]);
        Poly* b = polyFromBytes(k, &secret[i + k - 2]);

        uint8_t random = (rand() % (MOD - 2)) + 1;

        b->coef[0] = TO_POSITIVE(-(TO_POSITIVE(random) * secret[i]));
        b->coef[1] = TO_POSITIVE(-(TO_POSITIVE(random) * secret[i + 1]));

        secret[i] = auxa0;
        secret[i + 1] = auxa1;

        //save shadows 
        for (int j = 0; j < n; j++) {
            shadows[j][shadowPoints] = evaluatePolynomial(a, j + 1);
            shadows[j][shadowPoints + 1] = evaluatePolynomial(b, j + 1);
        }

        freePoly(a);
        freePoly(b);
    }
    return;
}

//-------------------------------------------------
//--------HELPER FUNCTIONS-------------------------
//-------------------------------------------------

bool validCoefficients(Poly* a, Poly* b) {
    for (int i = 0; i < MOD; i++) {
        if (TO_POSITIVE(a->coef[0] * i + b->coef[0]) == 0 && TO_POSITIVE(a->coef[1] * i + b->coef[1]) == 0) {
            return true;
        }
    }
    freePoly(a);
    freePoly(b);
    return false;
}
