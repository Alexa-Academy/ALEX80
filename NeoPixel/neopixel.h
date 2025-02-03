#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#define NUM_PIXEL 64

void neopixel_setPixelColor(unsigned int pixel, unsigned char red, unsigned char green, unsigned char blue);
void neopixel_setPixelColor_rgb(unsigned int pixel, long rgb);
unsigned long neopixel_getPixelColor(unsigned int pixel);
void neopixel_getPixelColor2(unsigned int pixel, unsigned char *red, unsigned char *green, unsigned char *blue);
void neopixel_show();
void neopixel_clear();

#endif // NEOPIXEL_H