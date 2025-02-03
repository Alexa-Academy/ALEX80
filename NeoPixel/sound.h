#ifndef SOUND_H
#define SOUND_H

#include <stdlib.h>

void beep_tone(uint16_t freq, uint16_t duration_ms);

void playVictoryTune();
void playLostTune();

#endif // SOUND_H