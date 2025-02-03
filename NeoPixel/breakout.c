#include <stdio.h>
#include <stdlib.h>
#include "breakout.h"
#include "neopixel.h"
#include <math.h>
#include "sound.h"

#define FREQ_PADDLE    800
#define DURATION_PADDLE  80

#define FREQ_WALL     1200
#define DURATION_WALL   80

#define FREQ_BRICK    2000
#define DURATION_BRICK 100


#define NUM_PIXELS 64
#define WIDTH 8
#define HEIGHT 8

#define BALL_SPEED 10

extern unsigned int ref[];

int block_count = 16;

// Game objects
struct Point {
    float x;
    float y;
};

struct Point ball;
struct Point ball_dir;
int paddle_pos;
unsigned char blocks[2][8];
unsigned char ball_speed_counter = 0;

void ball_start() {
    // Initialize ball position
    //ball.x = (float)WIDTH / 2.0f;
    //ball.y = (float)HEIGHT - 2.0f; 
    ball.x = 3.0f;
    ball.y = HEIGHT-2.0f;

    // Initialize ball direction
    ball_dir.x = 0.5f;
    ball_dir.y = -1;
}

void paddle_start() {
    // Initialize paddle
    paddle_pos = (float)(WIDTH / 2 - 2);
}

void init_game() {
    ball_start();
    
    paddle_start();
    
    // Initialize blocks
    for(int row = 0; row < 2; row++) {
        for(int col = 0; col < 8; col++) {
            blocks[row][col] = 1;
        }
    }
    for(int row = 2; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            blocks[row][col] = 0;
        }
    }

    block_count = 16;
}

// Restituisce un piccolo valore casuale in [-0.1, 0.1]
float randomSmallOffset() {
    // rand() % 3 - 1 può dare -1, 0, +1
    // Moltiplicato per 0.1 diventa -0.1, 0, +0.1
    // Volendo, si può fare di meglio con rand() e RAND_MAX.
    return (float)((rand() % 3) - 1) * 0.1f;
}

// Muove il paddle (controlla i limiti 0..(COLS-1)).
void movePaddle(int direction) {
    // direction = -1 (sinistra), +1 (destra), 0.0 (resta)
    // Potresti anche passare valori più piccoli per movimenti più precisi.
    paddle_pos += direction;

    // Evita che il paddle esca dai bordi.
    // Paddle lungo 3, quindi l'estremo destro è paddle.x + 2.
    if(paddle_pos < 0) {
        paddle_pos = 0;
    }
    if(paddle_pos > (WIDTH - 3)) {
        paddle_pos = (WIDTH - 3);
    }
}


// Aggiorna la posizione della pallina e gestisce collisioni
void updateBall() {
    // Memorizziamo vecchie posizioni (se servissero)
    float oldX = ball.x;
    float oldY = ball.y;

    // Spostiamo la pallina
    ball.x += ball_dir.x;
    ball.y += ball_dir.y;

    // Controllo collisioni con i bordi orizzontali (sinistra/destra)
    if(ball.x < 0.0f) {
        ball.x = 0.0f;
        ball_dir.x = -ball_dir.x; // inverte direzione
        beep_tone(FREQ_WALL, DURATION_WALL);
    } else if(ball.x >= (float)WIDTH) {
        ball.x = (float)(WIDTH - 1);
        ball_dir.x = -ball_dir.x;
        beep_tone(FREQ_WALL, DURATION_WALL);
    }

    // Controllo collisione con bordo in alto
    if(ball.y < 0.0f) {
        beep_tone(FREQ_WALL, DURATION_WALL);
        ball.y = 0.0f;
        ball_dir.y = -ball_dir.y;
    }

    // Se la pallina esce sotto, game over o pallina persa
    if(ball.y >= (float)HEIGHT) {
        // Re-inizializza o gestisci vite, punteggio, ecc.
        //ball.x = (float)WIDTH / 2.0f;
        //ball.y = (float)HEIGHT - 2.0f;
        //ball_dir.x = 0.5f;
        //ball_dir.y = -1.0f;

        ballLost();

        playLostTune();
    }

    // Controllo collisione con il paddle
    // Il paddle è lungo 3 celle: [paddle.x, paddle.x+1, paddle.x+2].
    // La riga del paddle è (int) paddle.y
    int py = HEIGHT - 1; //(int)paddle.y; 
    int pxStart = paddle_pos;
    int pxEnd   = pxStart + 2; 

    if((int)ball.y == py && 
       (int)ball.x >= pxStart && 
       (int)ball.x <= pxEnd) 
    {
        // Rimbalzo sul paddle
        ball_dir.y = -ball_dir.y;

        // Piccola variazione casuale su dx, per rompere traiettorie ripetitive
        ball_dir.x += randomSmallOffset();

        // Spostiamo la pallina appena sopra il paddle
        ball.y = HEIGHT - 1.0f; 

        beep_tone(FREQ_PADDLE, DURATION_PADDLE);
    }

    // Controllo collisione con i mattoncini
    // Ora che ball.x e ball.y sono float, facciamo il cast a int.
    int bx = (int)floor(ball.x);  
    int by = (int)floor(ball.y);

    //printf("Posizione pallina: %.2f, %.2f     (%d, %d)\n", ball.x, ball.y, bx, by);

    // Verifichiamo che siano dentro la griglia
    if(by >= 0 && by < WIDTH && bx >= 0 && bx < WIDTH) {
        if (blocks[by][bx] == 1) {
            blocks[by][bx] = 0; // rompi il mattoncino
            // Inverti la direzione in verticale
            ball_dir.y = -ball_dir.y;

            // Eventualmente si può aggiungere un piccolo offset anche qui:
            ball_dir.x += randomSmallOffset();

            // Decrementa il contatore dei mattoncini
            block_count--;
            //printf("Mattoncino rimosso, ne restano %d\n", block_count);

            beep_tone(FREQ_BRICK, DURATION_BRICK);
        }
    }

    if (block_count <= 0) {
        // Fine del gioco, hai vinto!
        playVictoryTune();
        noBlocks();
    }
}

/*
void update_game(char input) {
    // Move paddle based on input
    if(input == 'a' && paddle_pos > 0) paddle_pos--;
    if(input == 'd' && paddle_pos < 6) paddle_pos++;
    
    // Aggiorna la posizione della palla solo ogni 4 frames
    ball_speed_counter++;
    if(ball_speed_counter >= BALL_SPEED) {
        ball_speed_counter = 0;
        
        // Update ball position
        ball.x += ball_dir.x;
        ball.y += ball_dir.y;
        
        // Ball collision with walls
        if(ball.x <= 0 || ball.x >= 7) ball_dir.x = -ball_dir.x;
        if(ball.y <= 0) ball_dir.y = -ball_dir.y;
        
        // Ball collision with paddle
        if(ball.y >= 7) {
            if (ball.x == paddle_pos) {
                ball_dir.y = -ball_dir.y;
                ball_dir.x = -ball_dir.x;
            } else if (ball.x == paddle_pos - 1 || ball.x == paddle_pos + 1) {
                ball_dir.y = -ball_dir.y;
                ball_dir.x = -ball_dir.x;
            } else {
                // Game over - reset ball
                ball.x = 4;
                ball.y = 4;
                game_over = 1;
            }
        }
        
        // Ball collision with blocks
        if(ball.y < 2) {
            if(blocks[ball.y][ball.x]) {
                blocks[ball.y][ball.x] = 0;
                ball_dir.y = -ball_dir.y;
            }
        }
    }
}
*/

void draw_game() {
    // Clear display
    //neopixel_clear();
    for (int i = 0; i < NUM_PIXELS; i++) {
        neopixel_setPixelColor(ref[i], 0, 0, 0);
    }
    
    // Draw blocks
    for(int row = 0; row < 2; row++) {
        for(int col = 0; col < 8; col++) {
            if(blocks[row][col]) {
                neopixel_setPixelColor(ref[row * WIDTH + col], 0, 255, 0); // Green blocks
            }
        }
    }
    
    // Draw paddle
    int px = paddle_pos;
    int py = HEIGHT - 1;
    for(int i = 0; i < 3; i++) {
        neopixel_setPixelColor(ref[py * WIDTH + px + i], 255, 0, 0); // red paddle
    }
    
    // Draw ball
    neopixel_setPixelColor(ref[(int)ball.y * WIDTH + (int)ball.x], 255, 255, 255); // White ball

    // Update display
    neopixel_show();
}

