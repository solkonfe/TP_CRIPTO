#include "../include/secret.h"


uint8_t fourSignificant[] = {0xC0, 0x30, 0x0C, 0x03};
uint8_t twoSignificant[] = {0xF0, 0x0F};

bool validCoefficients(Poly* a, Poly* b);
char** makePath(struct params * params);
void writeFile(bmpFile * file, int fd);

static shadow * fromImageToShadow(uint8_t k , bmpFile * imageFile){
    shadow * shadow = malloc(sizeof (shadow));
    shadow->shadowNumber = imageFile->header->reserved1;
    shadow->pointNumber = imageFile->header->image_size_bytes / (k -1);
    shadow->points = calloc(shadow->pointNumber,1);

    int lsb4 = LSB4(k);
    int ImageBytesToShadowByte= LSB4_BYTES(lsb4);
    int bitOperator = lsb4 ? 0x0f:0x03;
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

static void insertBits(uint8_t  * image, uint8_t  * shadow, uint8_t  k){
    int lsb4 = LSB4(k);
    int bytes = LSB4_BYTES(lsb4);

    uint8_t  lsb4Shifter[] = {4, 0};
    uint8_t  lsb2Shifter[] = {6,4,2, 0};
    int current = 0 ;

    uint8_t  bits[bytes];

    for(int i = 0; i < bytes ; i++){
        bits[i] = lsb4 ? *shadow & twoSignificant[i] : *shadow & fourSignificant[i] ;
        bits[i] = lsb4 ? bits[i] >> lsb4Shifter[current] : bits[i] >>lsb2Shifter[current];
        current++;
    }

    int and = lsb4 ? 0xF0 : 0xFC;
    for (int i = 0 ; i < bytes ; i++)
        image[i] = (image[i] & and) + bits[i];

}

void hideShadow(uint8_t  k , bmpFile * file, shadow * hidingShadow){
    file->header->reserved1 = hidingShadow->shadowNumber;
    uint8_t * image = file->pixels;
    uint8_t * shadowPointer = hidingShadow->points;
    for(uint32_t  i = 0; i < hidingShadow->pointNumber; i++ ){
            insertBits(image, shadowPointer, k);
            shadowPointer += 1 ;
            image += (k == 3 || k == 4) ? 2 : 4;
    }
}

void hideSecret(int n, int k, char** fileNames, shadow ** shadows){
    for (int i = 0 ; i < n ; i ++){
        bmpFile  * currentImageFile = openBmpFile(fileNames[i]);
        shadow * currentShadow = shadows[i];
        hideShadow(k,currentImageFile, currentShadow);

        writeFile(currentImageFile, currentImageFile->fd);
    }
}

void distribute(struct params *  params) {
    bmpFile* file = openBmpFile(params->file);
    char ** fileNames = makePath(params);
    shadow ** shadows = generateShadows(file, params->k, params->n);
    hideSecret(params->n, params->k, fileNames, shadows);
}

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

void writeFile(bmpFile * file, int fd){
    int size = file->header->size - file->header->image_size_bytes;
    lseek(fd, 0, SEEK_SET);
    write(fd, file->header, size);
    write(fd, file->pixels, file->header->image_size_bytes);
    close(fd);
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

    int fd = open(params->file, O_WRONLY | O_CREAT);
    writeFile(file, fd);
}

shadow ** generateShadows(bmpFile * file, int k, int n) {
    uint32_t shadowPoints = (file->header->image_size_bytes) / (k - 1);
    shadow ** shadows = malloc(n * (sizeof(shadow *)));
    for (int i=0; i< n; i++){
        shadows[i]->shadowNumber = i+1;
        shadows[i]->pointNumber = shadowPoints;
        shadows[i]->points = malloc(shadowPoints * sizeof (uint8_t));
    }
    uint8_t * pixels = file->pixels;
    uint32_t current = 0;
    uint8_t* ys1 = malloc(k * sizeof(uint8_t));
    uint8_t* ys2 = malloc(k * sizeof(uint8_t));

    for (current= 0; current < shadowPoints; current += 2) {
        memcpy(ys1, pixels, k);
        memcpy(ys2 + 2, pixels + k, k-2);
        uint8_t random = (rand() % (MOD - 2)) + 1;

        int auxa0 = TO_POSITIVE(ys1[0]) == 0 ? 1: ys1[0];
        int auxa1 = TO_POSITIVE(ys1[1]) == 0 ? 1: ys1[1];

        ys2[0] = TO_POSITIVE(-(TO_POSITIVE(random) * auxa0));
        ys2[1] = TO_POSITIVE(-(TO_POSITIVE(random) * auxa1));

        //save shadows 
        for (int j = 0; j < n; j++) {
            shadows[j]->points[current] = evaluatePoly(k, ys1, shadows[j]->shadowNumber);
            shadows[j]->points[current + 1] = evaluatePoly(k, ys2, shadows[j]->shadowNumber);
        }

        pixels += (2*k) - 2;
    }
    return shadows;
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
