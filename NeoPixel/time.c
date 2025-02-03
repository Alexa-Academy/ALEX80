#include "time.h"

void delay(unsigned long tenth_ms) {
    unsigned long current_time = get_elapsed_time();
    while (get_elapsed_time() < current_time + tenth_ms) {}
}

void delay_loop(unsigned int duration) {
    for (unsigned int i=0; i<duration; ++i){}
}

// Delay “a millisecondi grossolani”
void delay_loop_ms(unsigned int ms)
{
    // Con i=6 ottieni ~1 ms, misurato sperimentalmente
    for (unsigned long i = 0; i < ms * 5; i++) {
        // loop vuoto
    }
}