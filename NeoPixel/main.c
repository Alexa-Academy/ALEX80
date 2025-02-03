// Codice per computer ALEX80 che permette il pilotaggio di LED NeoPixel
// con un po' di hardware esterno
// Vedere video YouTube https://youtube.com/live/eqKhF6nzU44
//
// Compilato con z88dk https://github.com/z88dk/z88dk
//
// Per generare il file binario a.out
// zcc +z80 -clib=classic main.c breakout.c neopixel.c sound.c time.c letters.c hardware_new.asm  -create-app -m -lm
// -lm aggiunge le librerie matematiche per usare i float
// 
// Per ottenere il listing del codice macchina
// z88dk-dis -o 0x8000 -x a.map a.rom > a.out

#pragma output CRT_ORG_CODE = 0x8000
#pragma output CRT_REGISTER_SP = 0x0000
   
#pragma output CRT_MODEL = 0       // RAM model
#pragma output CRT_ORG_DATA = 0    // DATA appends to CODE
#pragma output CRT_ORG_BSS = 0     // BSS appends to DATA

#include <stdio.h>
#include <stdlib.h>
#include "neopixel.h"
#include "breakout.h"
#include "sound.h"
#include "time.h"
#include "letters.h"

#define MATRIX_WIDTH 8      // Larghezza della matrice
#define MATRIX_HEIGHT 8     // Altezza della matrice
#define NUMPIXELS MATRIX_WIDTH*MATRIX_HEIGHT // Numero totale di LED 

unsigned char game_started = 0;
unsigned char game_over = 0;

unsigned char input_a();
void init_hw();

unsigned long logoMatrix[] = {
  0xff0000, 0xff0000, 0xff0000, 0xff0000, 0x000000, 0x000000, 0x000000, 0x000000,
  0xff0000, 0xff0000, 0x000000, 0x00aaaa, 0x00aaaa, 0x00aaaa, 0x000000, 0x000000,
  0xff0000, 0x000000, 0x00aaaa, 0x000000, 0x000000, 0x000000, 0x00aaaa, 0x000000,
  0xff0000, 0x00aaaa, 0x000000, 0x888888, 0x000000, 0x888888, 0x000000, 0x00aaaa,
  0x000000, 0x00aaaa, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x00aaaa,
  0x000000, 0x00aaaa, 0x000000, 0x888888, 0x000000, 0x888888, 0x000000, 0x00aaaa,
  0x000000, 0x000000, 0x00aaaa, 0x000000, 0x000000, 0x000000, 0x00aaaa, 0x000000,
  0x000000, 0x000000, 0x000000, 0x00aaaa, 0x00aaaa, 0x00aaaa, 0x000000, 0x000000,
};

unsigned long tree[] = {
  0x000000, 0x000000, 0x000000, 0x000500, 0x000500, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0x005000, 0x005000, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000500, 0x00ff00, 0x00ff00, 0x000500, 0x000000, 0x000000, 
  0x000000, 0x002000, 0x005000, 0x00ff00, 0x00ff00, 0x005000, 0x002000, 0x000000, 

  0x000500, 0x00ff00, 0x00ff00, 0x00ff00, 0x00ff00, 0x00ff00, 0x00ff00, 0x000500, 
  0x005000, 0x00ff00, 0x00ff00, 0x808000, 0x808000, 0x00ff00, 0x00ff00, 0x005000, 
  0x000000, 0x000000, 0x000000, 0x808000, 0x808000, 0x000000, 0x000000, 0x000000, 
  0x000000, 0x000000, 0x000000, 0xffff00, 0xffff00, 0x000000, 0x000000, 0x000000
};

unsigned int ref[] = {
   0,  7,  8, 15, 16, 23, 24, 31,
   1,  6,  9, 14, 17, 22, 25, 30,
   2,  5, 10, 13, 18, 21, 26, 29,
   3,  4, 11, 12, 19, 20, 27, 28,
  60, 59, 52, 51, 44, 43, 36, 35,
  61, 58, 53, 50, 45, 42, 37, 34,
  62, 57, 54, 49, 46, 41, 38, 33,
  63, 56, 55, 48, 47, 40, 39, 32 
};


// Funzione per impostare i LED in base alla matrice
void setMatrix(unsigned long* mat) {
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int pixelIndex = y * MATRIX_WIDTH + x;
      neopixel_setPixelColor_rgb(ref[pixelIndex], mat[pixelIndex]);
    }
  }
  neopixel_show();
} 


void twinklingEffect() {
  for (unsigned int i = 0; i < NUM_PIXEL; i++) {
    if ((rand() % 11) > 7) { // Accendi casualmente alcuni LED
      uint8_t r = (rand() % 205) + 50; // Rosso con intensità casuale
      uint8_t g = (rand() % 205) + 50; // Verde con intensità casuale
      uint8_t b = (rand() % 205) + 50; // Blu con intensità casuale
      neopixel_setPixelColor(ref[i], r, g, b);
      
    } else {
      neopixel_setPixelColor(ref[i], 0, 0, 0); // Spegni il LED
    }
  }
  neopixel_show();
}

void fadeOut() {
  for (int brightness = 255; brightness >= 0; brightness -= 10) {
   // printf("brightness: %d\n", brightness);

    for (unsigned int i = 0; i < NUMPIXELS; i++) {
      unsigned long color = neopixel_getPixelColor(i);
      unsigned char r = (color >> 16) & 0xFF;
      unsigned char g = (color >> 8) & 0xFF;
      unsigned char b = color & 0xFF;

      /*
      unsigned char r; 
      unsigned char g; 
      unsigned char b;

      neopixel_getPixelColor2(i, &r, &g, &b);*/

      // Riduci l'intensità di ciascun colore
      unsigned char r1 = (r * brightness) / 255;
      unsigned char g1 = (g * brightness) / 255;
      unsigned char b1 = (b * brightness) / 255;

    //  if (!(r == 0 && g == 0 && b == 0)) {
    //    printf("i: %d)  r: %x, g: %x, b: %x    r1: %x, g1: %x, b1: %x\n", i, r, g, b, r1, g1, b1);
    //  }

      neopixel_setPixelColor(i, r1, g1, b1);
    }
    neopixel_show();
  }
}

//unsigned int i;
void blink() {
    for (unsigned int i=0; i<NUM_PIXEL; ++i) {
        neopixel_setPixelColor(i, 0, 0, 255);
    }
    neopixel_show();
    delay(5000);
    neopixel_clear();
    delay(5000);
}

void ballLost() {
    game_started = 0;
    ball_start();
    paddle_start();
}

void noBlocks() {
  game_started = 0;
  game_over = 1;

  neopixel_clear(NUM_PIXEL);
}

unsigned long elapsed_time;

int main() {
/*
  // Sequenza di animazioni natalizie
  init_hw();

  srand((unsigned int)get_elapsed_time());

  while (1) {
    // Scritta che scorre
    scrollText(" ALEXA ACADEMY", 100); 

    // Logo alexa academy
    for (int i=0; i<2; ++i) {
      setMatrix(logoMatrix);
      delay(100);
      fadeOut();
    }

    // Luci casuali
    for (unsigned int i = 0; i < 20; i++) { // Ripeti l'animazione 20 volte
        twinklingEffect();
        delay(8); // Breve pausa per ogni ciclo
    }
    fadeOut();   

    // Albero di Natale
    for (int i=0; i<3; ++i) {
      setMatrix(tree);
      delay(100);
      fadeOut();
    }

    // Scritta che scorre
    scrollText(" BUON NATALE E FELICE 2025", 100); 
  }
  */
  
  // Visualizza luci casuali
  /*
  srand((unsigned int)get_elapsed_time());
  while (1) {
      // random matrix
      for (int i=0; i<NUM_PIXEL; ++i) {
        neopixel_setPixelColor(ref[i], rand() % 256, rand() % 256, rand() % 256);
      }
      neopixel_show();

      delay_loop(10000);

      neopixel_clear(NUM_PIXEL);

      delay_loop(100);
  }
  */


  // Gioco breakout
  game_over = 0;
  game_started = 0;

  init_hw();

  srand((unsigned int)get_elapsed_time());

  init_game();

  playVictoryTune();

  unsigned char in=0;
  while (1) {
    if (!game_over) {
      draw_game();

      if (game_started) updateBall();

      in = input_a();
      if ((in&1) == 1) {
        movePaddle(-1);
        game_started = 1;
      } else if ((in&2) == 2) {
        movePaddle(1);
        game_started = 1;
      }

      delay_loop(50);
    }
  }
}
