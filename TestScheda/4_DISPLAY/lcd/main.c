// zcc +z80 -clib=classic main.c liquid.c hardware.asm  -create-app -m
// z88dk-dis -o 0x8000 -x a.map a.rom

#pragma output CRT_ORG_CODE = 0x8000
#pragma output CRT_REGISTER_SP = 0x0000

#include <stdio.h>
#include "liquid.h"

const int rs = 0x04, en = 0x08, d4 = 0x10, d5 = 0x20, d6 = 0x40, d7 = 0x80;

char buffer[10];

int counter = 0;

int main() {
    LiquidCrystal_c4(rs, en, d4, d5, d6, d7);
    LiquidCrystal_begin(16, 2, LCD_5x8DOTS);
    //LiquidCrystal_begin(20, 4, LCD_5x8DOTS);

    char *str_hello = "Ciao da ALEX80!";
    write_string(str_hello, strlen(str_hello));

    while (1) {
        LiquidCrystal_setCursor(0, 1);
        sprintf(buffer, "%d", counter++);
        write_string(buffer, strlen(buffer));
        for (int i = 0; i < 1000; i++);
    }
}
