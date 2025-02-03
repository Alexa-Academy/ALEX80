#include "sound.h"
#include "time.h"

#define BUZZER_PORT 0x0C

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

typedef struct {
    uint16_t freq;       // frequenza in Hz
    uint16_t duration;   // durata in millisecondi
} note_t;

// Esempio di musichetta “vittoria”
static const note_t victoryTune[] = {
    { NOTE_A5, 200 },  
    { NOTE_B5, 200 },  
    { NOTE_C5, 200 },  
    { NOTE_B5, 200 }, 
    { NOTE_C5, 200 },
    { NOTE_D5, 200 },
    { NOTE_C5, 200 },
    { NOTE_D5, 200 },
    { NOTE_E5, 200 },
    { NOTE_D5, 200 },
    { NOTE_E5, 200 },
    { NOTE_E5, 200 },
    { 0, 0 }           // Terminatore (freq = 0, durata = 0)
};

// Toggles pin a 'freq' Hz per 'duration_ms' millisecondi (approssimazioni).
void beep_tone(uint16_t freq, uint16_t duration_ms) {
    // Durata di un periodo in microsecondi
    //uint16_t period_us = (uint16_t)(1000000UL / freq);
    //uint16_t period_ms = (uint16_t)((1000000UL / freq) * 1000);
    unsigned int halfPeriodMs = 500 / freq;
    if (halfPeriodMs == 0) {
        halfPeriodMs = 1;  // evita divisioni nulle, almeno 1ms
    }

    // Quanti cicli in 'duration_ms'?
    // freq Hz = freq cicli al secondo -> freq * (duration_ms/1000) cicli totali
    uint16_t total_cycles = (freq * duration_ms) / 1000;
    
    while(total_cycles--) {
        // Porta a 1
        outp(BUZZER_PORT, 0x01);
        delay_loop_ms(halfPeriodMs);
       // z80_delay_us(period_us/2); // Prima metà periodo

        // Porta a 0
        outp(BUZZER_PORT, 0x00);
        delay_loop_ms(halfPeriodMs);
       // z80_delay_us(period_us/2); // Seconda metà periodo
    }
}

void playTune(const note_t *tune) {
    int i = 0;
    while (tune[i].freq != 0 || tune[i].duration != 0) {
        beep_tone(tune[i].freq, tune[i].duration);
        i++;
        //delay_loop(100);
    }
}

void playVictoryTune() {
    playTune(victoryTune);
}

void playLostTune() {
    beep_tone(NOTE_G4, 400);
    delay_loop(100);
    beep_tone(NOTE_C4, 400);
    delay_loop(100);
}