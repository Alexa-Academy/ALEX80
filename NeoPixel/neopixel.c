#include "neopixel.h"

// Definite in hardware.asm
void neopixel_hw_send(unsigned char *m, unsigned int num_pixel);
void neopixel_hw_clear(unsigned int num_pixel);

char neopixel_matrix[NUM_PIXEL*3];


void neopixel_setPixelColor(unsigned int pixel, unsigned char red, unsigned char green, unsigned char blue) {
    neopixel_matrix[pixel*3] = red;
    neopixel_matrix[(pixel*3)+1] = green;
    neopixel_matrix[(pixel*3)+2] = blue;
}

void neopixel_setPixelColor_rgb(unsigned int pixel, long rgb) {
    neopixel_matrix[pixel*3] = (rgb >> 16) & 0xFF;
    neopixel_matrix[(pixel*3)+1] = (rgb >> 8) & 0xFF;
    neopixel_matrix[(pixel*3)+2] = rgb & 0xFF;
}

unsigned long neopixel_getPixelColor(unsigned int pixel) {
    unsigned char r = neopixel_matrix[pixel*3];
    unsigned char g = neopixel_matrix[(pixel*3)+1];
    unsigned char b = neopixel_matrix[(pixel*3)+2];

    return ((unsigned long)r << 16) | ((unsigned long)g << 8) | b;
}

void neopixel_getPixelColor2(unsigned int pixel, unsigned char *red, unsigned char *green, unsigned char *blue) {
    *red = neopixel_matrix[pixel*3];
    *green = neopixel_matrix[(pixel*3)+1];
    *blue = neopixel_matrix[(pixel*3)+2];
}

void neopixel_show() {
    neopixel_hw_send(neopixel_matrix, NUM_PIXEL);
}

void neopixel_clear() {
    neopixel_hw_clear(NUM_PIXEL);
    for (unsigned int i = 0; i < NUM_PIXEL*3; i++) {
        neopixel_matrix[i] = 0;
    }
}
