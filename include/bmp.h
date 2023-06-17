#ifndef BMP_H
#define BMP_H

#include <stdint.h>


//https://codereview.stackexchange.com/questions/196084/read-and-write-bmp-file-in-c

typedef struct  __attribute__((__packed__)) BMPHeader {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
    uint32_t dib_header_size;   // DIB Header size in bytes
    int32_t  width_px;          // Width of the image
    int32_t  height_px;         // Height of image
    uint16_t num_planes;        // Number of color planes
    uint16_t bits_per_pixel;    // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size_bytes;  // Image size in bytes
    int32_t  x_resolution_ppm;  // Pixels per meter
    int32_t  y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;        // Number of colors
    uint32_t important_colors;  // Important colors
} BMPHeader;

//header 54 bytes
//data: 8 bits per image: 0x00 is black and 0xFF is white
typedef struct BMPImage {
    BMPHeader * header; 
    uint8_t * data;
} BMPImage;

typedef struct bmpFile{
    BMPHeader  * header;
    uint8_t * pixels;
    int fd ;
} bmpFile;



//used to parse a bmp file.
bmpFile *  openBmpFile(const char * path);

BMPImage * readBmp (const char * path);

void freeBmp(BMPImage *image);

void writeToFileBmpImage (BMPImage * bmp, char * path);

void writeToStdoutBmpImage (BMPImage * bmp);

BMPImage * writeImage (BMPHeader * header, uint8_t * data);

#endif
