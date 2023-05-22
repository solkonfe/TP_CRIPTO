#include "../include/bmp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC_VALUE 0x4D42

BMPHeader * copyHeader (BMPHeader * header) {
    BMPHeader * final = malloc(sizeof(BMPHeader));
    memcpy(final, header, sizeof(BMPHeader));
    return final;
}

BMPImage * readBmp (const char * path){
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Failed openning fd");
        return NULL;
    }

    struct stat fileStatus;
    if (fstat(fd, &fileStatus) == -1) {
        perror("Failed the fstat. We will close the fd.");
        close(fd);
        return NULL;
    }

    uint8_t* map = (uint8_t*) mmap(NULL, fileStatus.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Failed during the mapping. We will close the fd.");
        close(fd);
        return NULL;
    }

    BMPHeader* bmpHeader = (BMPHeader*) map;

    if (bmpHeader->type != MAGIC_VALUE) {
        fprintf(stderr, "Invalid magic identifier for BMP Header. We will close the fd\n");
        munmap(map, fileStatus.st_size);
        close(fd);
        return NULL;
    }

    if (bmpHeader->bits_per_pixel != 8) {
        fprintf(stderr, "Invalid bits per pixel. We will close the fd");
        munmap(map, fileStatus.st_size);
        close(fd);
        return NULL;
    }

    BMPImage * bmpImage = malloc(sizeof(BMPImage)); 

    uint8_t* data = map+bmpHeader->offset;
    bmpImage->data = data; 
    bmpImage->header = bmpHeader; 

    return bmpImage;
}

void freeBmp(BMPImage *image) {
    if (image) {
      free(image->data);
      free(image);
    } 
}
